#include "tm_thread_events_queue.h"

#include "tm_alloc.h"
#include "tm_block.h"
#include "tm_configuration.h"
#include "tm_logging.h"
#include "tm_time.h"
#include "tm_queue.h"

#include <ev.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stddef.h>
#include <math.h>
#include <tm_block.h>

static int signals[] = {
		SIGINT,
		SIGTERM,
		SIGQUIT,
		SIGTSTP,
		SIGPWR
};

#define SIGNAL_COUNT (sizeof(signals) / sizeof(*signals))

/**
 * Структура/контекст рабочего потока.
 */
typedef struct {
	// Общая часть
	pthread_t thread_id;          // id рабочего потока
	ev_async w_pointers_in_queue; // watcher на сигнал от главного потока о том, что он положил в очередь указатели
	pthread_mutex_t queue_mutex;
	tm_queue_ctx *queue;
	volatile size_t pointers_in_queue_count;

	// Часть рабочего потока
	struct ev_loop *loop;    // петля рабочего потока
	ev_async w_shutdown;     // wathcer на завершение потока
	tm_time_bitrate *bitrate;// массив контекстов для расчёта битрейта клиентов рабочего потока
	size_t clients_count;    // количество клиентов, обслуживаемых потоком
} tm_work_thread_t;

/**
 * Структура/контекст главного потока
 */
typedef struct {
	struct ev_loop *loop;              // петля главного потока
	ev_timer w_test_time;              // watcher на таймер, который установлен на длительность теста
	ev_signal w_signals[SIGNAL_COUNT]; // watcher'ы на сигналы
	ev_async w_low_bitrate;            // watcher на сигнал о низком битрейте от рабочих потоков
	client_block_t *client_block_array;
	ev_timer w_next_send_start;// wathcer на таймер до следующей отправки на рабочий потоки

	int low_bitrate_flag;              // флаг, говорящий о том, что был низкий битрейт

	tm_work_thread_t *work_threads;    // контексты рабочих потоков
	size_t work_threads_count;         // количество рабочих потоков

} tm_main_thread_t;

static tm_main_thread_t main_thread;


static void pointers_callback(EV_P_ ev_async *w, int revents)
{
	(void)w; (void)revents; int first_time = 1;
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_pointers_in_queue));
	while (__sync_add_and_fetch(&thread->pointers_in_queue_count, 0) != 0) {
		size_t pop_count = 1000;
		size_t actual_pop_count = 0;
		client_block_t client_block_array[pop_count];

		if (first_time) {
			first_time = 0;
		} else {
			ev_now_update(loop);
		}

		if (ev_async_pending(&thread->w_shutdown))
			break;

		pthread_mutex_lock(&thread->queue_mutex);
		int pop_res = tm_queue_pop_front(thread->queue, client_block_array, (int) pop_count);
		pthread_mutex_unlock(&thread->queue_mutex);
		if (pop_res) {
			for (size_t i = 0; i < pop_count; i++) {
				if (client_block_array[i].block == NULL)
					break;

				size_t client_id = client_block_array[i].client_id;

				if (sample_bitrate(&thread->bitrate[client_id], ev_now(loop)))
				{
					if (thread->bitrate[client_id].bitrate <= (double) configuration.bitrate - configuration.bitrate_diff)
					{
						ev_async_send(main_thread.loop, &main_thread.w_low_bitrate);
					}
				}

				actual_pop_count++;
				tm_block_dispose_block(client_block_array[i].block);
			}
			__sync_sub_and_fetch(&thread->pointers_in_queue_count, actual_pop_count);
		}
	}
}

static void next_send_start_callback(EV_P_ ev_timer *w, int revents)
{
	(void)w; (void)revents;
	ev_timer_again(loop, w);
	//client_block_t client_block_array[main_thread.work_threads[0].clients_count];
	tm_block *block = tm_block_create();
	for (int i = 0; i < main_thread.work_threads_count; i++)
	{
		if (configuration.optimize_refcount_use_by_copy && i != 0) {
			tm_block *tmp = tm_block_copy(block);
			tm_block_dispose_block(block);
			block = tmp;
		}
		tm_work_thread_t *thread = &main_thread.work_threads[i];
		for (size_t j = 0; j < thread->clients_count; j++)
		{
			main_thread.client_block_array[j].block = tm_block_transfer_block(block);
			main_thread.client_block_array[j].client_id = j;
		}
		pthread_mutex_lock(&thread->queue_mutex);
		tm_queue_push_back(thread->queue, main_thread.client_block_array, (int) thread->clients_count);
		pthread_mutex_unlock(&thread->queue_mutex);
		__sync_add_and_fetch(&thread->pointers_in_queue_count, thread->clients_count);
		ev_async_send(thread->loop, &thread->w_pointers_in_queue);
	}
	tm_block_dispose_block(block);
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для завершения потока
 * Сидит на петле рабочего потока. Посылает главный поток.
 */
static void thread_shutdown_callback(EV_P_ ev_async *w, int revents)
{
	(void)w; (void)revents;
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_shutdown));

	ev_async_stop(loop, w);
	ev_async_stop(loop, &thread->w_pointers_in_queue);
	ev_break(loop, EVBREAK_ONE);
}

/**
 * Функция рабочего потока
 */
static void *work_thread_function(void *arg)
{
	tm_work_thread_t *thread = (tm_work_thread_t*)arg;

	ev_async_init(&thread->w_shutdown, thread_shutdown_callback);
	ev_async_start(thread->loop, &thread->w_shutdown);

	ev_async_init(&thread->w_pointers_in_queue, pointers_callback);
	ev_async_start(thread->loop, &thread->w_pointers_in_queue);

	ev_run(thread->loop, 0);
	return NULL;
}

static TMThreadStatus tm_thread_thread_create(tm_work_thread_t *thread)
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if (!thread)
		return status;

	if (thread->clients_count <= 0) {
		TM_LOG_ERROR("Attempt to start thread with <= 0 clients on it.");
		return status;
	}

	thread->bitrate = tm_calloc(thread->clients_count * sizeof(tm_time_bitrate));
	if (!thread->bitrate) {
		TM_LOG_ERROR("Failed to allocate memory for bitrate contexts.");
		return status;
	}

	for(int i = 0; i < thread->clients_count; i++)
		thread->bitrate[i].bitrate_sample_count = -1;

	pthread_mutex_init(&thread->queue_mutex, NULL);

	thread->queue = tm_queue_create(configuration.queue_type);
	if (!thread->queue) {
		return status;
	}

	thread->loop = ev_loop_new(0);
	if (!thread->loop) {
		TM_LOG_ERROR("Failed to create loop for work thread.");
		return status;
	}

	if (pthread_create(&thread->thread_id, NULL, work_thread_function, (void*)thread) != 0) {
		TM_LOG_ERROR("Failed to create thread.");
		return status;
	}

	status = TMThreadStatus_SUCCESS;
	return status;
}

static void tm_thread_shutdown(tm_work_thread_t *thread)
{
	if (thread) {
		if (thread->loop)
			ev_async_send(thread->loop, &thread->w_shutdown);
		pthread_join(thread->thread_id, NULL);
		tm_queue_destroy(thread->queue);
		pthread_mutex_destroy(&thread->queue_mutex);
		ev_loop_destroy(thread->loop);
		tm_free(thread->bitrate);
	}
}

static void stop_main_thread_watchers()
{
	for (int i = 0; i < SIGNAL_COUNT; i++) {
		ev_signal_stop(main_thread.loop, &main_thread.w_signals[i]);
	}
	ev_timer_stop(main_thread.loop, &main_thread.w_test_time);
	ev_timer_stop(main_thread.loop, &main_thread.w_next_send_start);
	ev_async_stop(main_thread.loop, &main_thread.w_low_bitrate);
}

static void signal_callback(EV_P_ ev_signal *w, int revents)
{
	(void)w; (void)revents;
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

static void test_time_callback(EV_P_ ev_timer *w, int revents)
{
	(void)w; (void)revents;
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для сигнализации главному потоку о низком битрейте.
 * Крутится на петле главного потока. Посылают рабочие потоки.
 */
static void low_bitrate_callback(EV_P_ ev_async *w, int revents)
{
	(void)w; (void)revents;
	main_thread.low_bitrate_flag = 1;
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

TMThreadStatus tm_threads_init_events_queue(int count)
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	size_t clients_count;

	if (count <= 0) {
		TM_LOG_ERROR("Attempt to create <= 0 work threads.");
		return status;
	}

	main_thread.work_threads = tm_calloc(count * sizeof(tm_work_thread_t));
	if (!main_thread.work_threads) {
		TM_LOG_ERROR("Failed to allocate memory for thread contexts.");
		return status;
	}

	main_thread.loop = EV_DEFAULT;
	ev_timer_init(&main_thread.w_test_time, test_time_callback, (double) configuration.test_time, 0.0);
	ev_timer_init(&main_thread.w_next_send_start, next_send_start_callback, configuration.sleep_time / 1.0e6, configuration.sleep_time / 1.0e6);

	for (int i = 0; i < SIGNAL_COUNT; i++) {
		ev_signal_init(&main_thread.w_signals[i], signal_callback, signals[i]);
		ev_signal_start(main_thread.loop, &main_thread.w_signals[i]);
	}

	ev_async_init(&main_thread.w_low_bitrate, low_bitrate_callback);
	ev_async_start(main_thread.loop, &main_thread.w_low_bitrate);

	clients_count = (size_t) configuration.clients_count / count;
	for(int i = 0; i < count; i++) {
		main_thread.work_threads[i].clients_count = clients_count;
	}
	for(int i = 0; i < configuration.clients_count % count; i++) {
		main_thread.work_threads[i].clients_count++;
	}

	main_thread.client_block_array = tm_alloc(sizeof(client_block_t) * main_thread.work_threads[0].clients_count);
	if (!main_thread.client_block_array) {
		return status;
	}

	main_thread.work_threads_count = 0;
	for(int i = 0; i < count; i++) {
		if (tm_thread_thread_create(&main_thread.work_threads[i]) != TMThreadStatus_SUCCESS) {
			TM_LOG_ERROR("Failed to create a work thread.");
			return status;
		}
		main_thread.work_threads_count++;
	}

	status = TMThreadStatus_SUCCESS;
	return status;
}

TMThreadStatus tm_threads_work_events_queue()
{
	ev_timer_start(main_thread.loop, &main_thread.w_test_time);
	ev_timer_again(main_thread.loop, &main_thread.w_next_send_start);
	ev_run(main_thread.loop, 0);

	if (main_thread.low_bitrate_flag) {
		TM_LOG_TRACE("Low bitrate");
		return TMThreadStatus_ERROR;
	}

	return TMThreadStatus_SUCCESS;
}

TMThreadStatus tm_threads_shutdown_events_queue()
{
	TMThreadStatus status = TMThreadStatus_SUCCESS;

	for (int i = 0; i < main_thread.work_threads_count; i++) {
		tm_thread_shutdown(&main_thread.work_threads[i]);
	}
	tm_free(main_thread.client_block_array);
	tm_free(main_thread.work_threads);
	ev_loop_destroy(main_thread.loop);

	return status;
}
