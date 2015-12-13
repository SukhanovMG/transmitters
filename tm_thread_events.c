#include "tm_thread_events.h"

#include "tm_alloc.h"
#include "tm_block.h"
#include "tm_configuration.h"
#include "tm_logging.h"

#include <ev.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stddef.h>
#include <math.h>

static int signals[] = {
		SIGINT,
		SIGTERM,
		SIGQUIT,
		SIGTSTP,
		SIGPWR
};

#define SIGNAL_COUNT (sizeof(signals) / sizeof(*signals))

/**
 * Структура сообщения, посылаемого через канал
 */
typedef struct {
	tm_block *block;  // указатель на блок
	size_t client_id; // id клиента, для которого предназначен блок
} pipe_msg_t;

// Количество сообщений, которое можно записть в канал атомарно
#define PIPE_MSG_ATOMIC_WRITE_COUNT (PIPE_BUF / sizeof(pipe_msg_t))
// Величина PIPE_MSG_ATOMIC_WRITE_COUNT в байтах
#define PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES (PIPE_MSG_ATOMIC_WRITE_COUNT * sizeof(pipe_msg_t))

// Размер массива указателей для возврата на главный поток
#define POINTERS_TO_SEND_BACK_ARRAY_SIZE (1024 * 1024)

#define PIPE_POINTERS_ATOMIC_WRITE_COUNT (PIPE_BUF / sizeof(tm_block*))
#define PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES (PIPE_POINTERS_ATOMIC_WRITE_COUNT * sizeof(tm_block*))

typedef struct _tm_time_bitrate {
	double bitrate;
	int bitrate_sample_count;
	double start_time;
} tm_time_bitrate;

/**
 * Структура/контекст рабочего потока.
 */
typedef struct {
	// Общая часть
	pthread_t thread_id;     // id рабочего потока
	int pipe_fd[2];          // файловые дескрипторы каналов для передачи указателей
	int pipe_fd_return[2];   // для возврата указателей

	// Часть рабочего потока
	struct ev_loop *loop;    // петля рабочего потока
	ev_async w_shutdown;     // wathcer на завершение потока
	ev_io w_pipe_read;       // wathcer на чтение из канала
	tm_time_bitrate *bitrate;// массив контекстов для расчёта битрейта клиентов рабочего потока
	size_t clients_count;    // количество клиентов, обслуживаемых потоком
	tm_block **pointers_to_send_back;
	int pointers_to_send_back_count;
	ev_io w_pipe_ret_ptrs_write;

	// Часть главного потока (относится к главному потоку, но нужно по одному экземпляру на клиента, поэтому здесь)
	tm_block *block_to_send; // указатель на блок, который в данный момент отправляется на рабочие потоки клиентам
	double write_start_time; // время начала отправки текущего блока на рабочие потоки клиентам
	size_t clients_serviced; // скольким клиентам уже отправлен текущий блок
	size_t blocks_sent;      // блоков всего отправлено
	ev_io w_pipe_write;      // wathcer на запись в канал
	ev_timer w_next_io_start;// wathcer на таймер до следующей активации watcher'а на запись в канал
	ev_io w_pipe_ret_ptrs_read;
} tm_work_thread_t;

/**
 * Структура/контекст главного потока
 */
typedef struct {
	struct ev_loop *loop;              // петля главного потока
	ev_timer w_test_time;              // watcher на таймер, который установлен на длительность теста
	ev_signal w_signals[SIGNAL_COUNT]; // watcher'ы на сигналы
	ev_async w_low_bitrate;            // watcher на сигнал о низком битрейте от рабочих потоков

	int low_bitrate_flag;              // флаг, говорящий о том, что был низкий битрейт

	tm_work_thread_t *work_threads;    // контексты рабочих потоков
	size_t work_threads_count;         // количество рабочих потоков

} tm_main_thread_t;

static tm_main_thread_t main_thread;


static double calc_bitrate(double t2, double t1, int samples)
{
	double diff = fabs(t2 - t1);
	return (double) configuration.block_size * 8.0 * samples / (diff == 0? 1e-9 : diff) / 1024.0;
}

static int sample_bitrate(tm_time_bitrate *bitrate_ctx, struct ev_loop *loop)
{
	int ret = 0;
	double cur_time = ev_now(loop);

	if (bitrate_ctx->bitrate_sample_count == -1)
	{
		bitrate_ctx->bitrate = 0.0;
		bitrate_ctx->start_time = cur_time;
		bitrate_ctx->bitrate_sample_count = 0;
	}
	else
	{
		bitrate_ctx->bitrate_sample_count++;
	}

	if (cur_time - bitrate_ctx->start_time >= configuration.avg_bitrate_calc_time)
	{
		ret = 1;
		if (bitrate_ctx->bitrate_sample_count != 0)
			bitrate_ctx->bitrate = calc_bitrate(cur_time, bitrate_ctx->start_time, bitrate_ctx->bitrate_sample_count);
		bitrate_ctx->bitrate_sample_count = -1;
	}
	return ret;
}


/**
 * Коллбэк для ev_timer watcher'a, который используется для запуска watcher'а на запись в пайп.
 * По одному на каждый рабочий поток. Крутятся на петле главного потока. Запускаются, когда мы достигли
 * нужной скорости, чтобы подождать до конца отсчётного периода битрейта.
 */
static void timer_for_next_io_callback(EV_P_ ev_timer *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_next_io_start));
	thread->write_start_time = 0.0;
	thread->blocks_sent = 0;
	ev_io_start(loop, &thread->w_pipe_write);
}

/**
 * Коллбэк для ev_io wathcer'а, который используется для записи в пайпы (по одному на рабочий поток).
 * Крутятся на петле главного потока. Вызывается, когда главный поток может писать в пайп.
 */
static void pipe_write_callback(EV_P_ ev_io *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_pipe_write));
	double bitrate;
	ssize_t write_res;
	pipe_msg_t messages[PIPE_MSG_ATOMIC_WRITE_COUNT] = {{NULL, 0}};
	size_t i = 0;

	if (thread->write_start_time == 0.0) {
		thread->write_start_time = ev_now(loop);
	}

	if (thread->clients_serviced == thread->clients_count) {
		tm_block_dispose_block(thread->block_to_send);
		thread->block_to_send = NULL;
		thread->clients_serviced = 0;
		thread->blocks_sent += 1;
		bitrate = thread->blocks_sent * configuration.block_size * 8.0 / 1024.0 / 1.0;
		/*
		if ((1.0 - (ev_now(loop) - thread->write_start_time)) < 0)
			TM_LOG_ERROR("=============");
		*/
		if ((ev_now(loop) - thread->write_start_time) >= 1.0 || bitrate >= configuration.bitrate) {
			ev_io_stop(loop, w);
			ev_timer_init(&thread->w_next_io_start, timer_for_next_io_callback, 1.0 - (ev_now(loop) - thread->write_start_time), 0.0);
			ev_timer_start(loop, &thread->w_next_io_start);
			return;
		}
	}

	if (!thread->block_to_send)
		thread->block_to_send = tm_block_create();

	for (i = 0; i + thread->clients_serviced < thread->clients_count && i < PIPE_MSG_ATOMIC_WRITE_COUNT; i++) {
		messages[i].block = tm_block_transfer_block(thread->block_to_send);
		messages[i].client_id = i + thread->clients_serviced;
	}

	if (i < PIPE_MSG_ATOMIC_WRITE_COUNT) {
		messages[i].block = NULL; // terminator (если массив сообщений не полностью забит)
	}

	write_res = write(thread->pipe_fd[1], (const void *) messages, PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES);
	if (write_res < 0) {
		TM_LOG_ERROR("Failed to write to pipe");
		return;
	} else if (write_res < PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES) {
		TM_LOG_ERROR("Failed to write %zu bytes (%zd written)", PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES, write_res);
		return;
	}

	thread->clients_serviced += i;
}

/**
 * Коллбэк для ev_io watcher'а, который используется для чтения из пайпа (по одному на рабочий поток).
 * Крутятся на петлях рабочих потоков. Вызываются, когда можем прочитать посланные
 * главным потоком данные.
 */
static void pipe_read_callback(EV_P_ ev_io *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_pipe_read));
	pipe_msg_t messages[PIPE_MSG_ATOMIC_WRITE_COUNT] = {{NULL, 0}};
	ssize_t read_res;

	read_res = read(thread->pipe_fd[0], (void *) &messages[0], PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES);
	if (read_res < 0) {
		TM_LOG_ERROR("Failed to read from a pipe");
		TM_LOG_TRACE("errno = %d", errno);
		return;
	} else if (read_res != PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES) {
		TM_LOG_ERROR("Failed to read %zu bytes (%zd red)", PIPE_MSG_ATOMIC_WRITE_COUNT_BYTES, read_res);
		return;
	}

	for (int i = 0; i < PIPE_MSG_ATOMIC_WRITE_COUNT; i++) {
		if (messages[i].block == NULL)
			break;

		if (configuration.return_pointers_through_pipes) {
		//if (0) {
			if (thread->pointers_to_send_back_count == POINTERS_TO_SEND_BACK_ARRAY_SIZE) {
				TM_LOG_ERROR("Pointers to send back overflow");
				return;
			}
			thread->pointers_to_send_back[thread->pointers_to_send_back_count] = messages[i].block;
			thread->pointers_to_send_back_count++;
		} else {
			tm_block_dispose_block(messages[i].block);
		}
		size_t client_id = messages[i].client_id;

		if (sample_bitrate(&thread->bitrate[client_id], loop))
		{
			if (thread->bitrate[client_id].bitrate <= (double) configuration.bitrate - configuration.bitrate_diff)
			{
				ev_async_send(main_thread.loop, &main_thread.w_low_bitrate);
			}
		}
	}
}

static void return_ptrs_write_callback(EV_P_ ev_io *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_pipe_ret_ptrs_write));
	tm_block *pointers_to_send[PIPE_POINTERS_ATOMIC_WRITE_COUNT] = {NULL};
	if (thread->pointers_to_send_back_count > 0) {
		ssize_t write_res;
		int i = 0;
		while(i < PIPE_POINTERS_ATOMIC_WRITE_COUNT && thread->pointers_to_send_back_count > 0) {
			pointers_to_send[i] = thread->pointers_to_send_back[thread->pointers_to_send_back_count - 1];
			thread->pointers_to_send_back_count--;
			i++;
		}
		if (i < PIPE_POINTERS_ATOMIC_WRITE_COUNT) {
			pointers_to_send[i] = NULL;
		}

		write_res = write(thread->pipe_fd_return[1], (const void *) pointers_to_send, PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES);
		if (write_res < 0) {
			TM_LOG_ERROR("Failed to write to pipe");
			return;
		} else if (write_res < PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES) {
			TM_LOG_ERROR("Failed to write %zu bytes (%zd written)", PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES, write_res);
			return;
		}
	}
}

static void return_ptrs_read_callback(EV_P_ ev_io *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_pipe_ret_ptrs_read));
	tm_block *pointers[PIPE_POINTERS_ATOMIC_WRITE_COUNT] = {NULL};
	ssize_t read_res;

	read_res = read(thread->pipe_fd_return[0], (void *) pointers, PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES);
	if (read_res < 0) {
		TM_LOG_ERROR("Failed to read from a pipe");
		TM_LOG_TRACE("errno = %d", errno);
		return;
	} else if (read_res != PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES) {
		TM_LOG_ERROR("Failed to read %zu bytes (%zd red)", PIPE_POINTERS_ATOMIC_WRITE_COUNT_BYTES, read_res);
		return;
	}

	for (int i = 0; i < PIPE_POINTERS_ATOMIC_WRITE_COUNT; i++) {
		if (pointers[i] == NULL) {
			break;
		}

		tm_block_dispose_block(pointers[i]);
	}
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для завершения потока
 * Сидит на петле рабочего потока. Посылает главный поток.
 */
static void thread_shutdown_callback(EV_P_ ev_async *w, int revents)
{
	tm_work_thread_t *thread = (tm_work_thread_t*) ((char*)w - offsetof(tm_work_thread_t, w_shutdown));

	ev_async_stop(loop, w);
	ev_io_stop(loop, &thread->w_pipe_read);
	if (configuration.return_pointers_through_pipes)
		ev_io_stop(loop, &thread->w_pipe_ret_ptrs_write);
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

	ev_io_init(&thread->w_pipe_read, pipe_read_callback, thread->pipe_fd[0], EV_READ);
	ev_io_start(thread->loop, &thread->w_pipe_read);

	if (configuration.return_pointers_through_pipes) {
		ev_io_init(&thread->w_pipe_ret_ptrs_write, return_ptrs_write_callback, thread->pipe_fd_return[1], EV_WRITE);
		ev_io_start(thread->loop, &thread->w_pipe_ret_ptrs_write);
	}

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

	if (configuration.return_pointers_through_pipes) {
		thread->pointers_to_send_back = tm_alloc(POINTERS_TO_SEND_BACK_ARRAY_SIZE * sizeof(tm_block *));
		if (!thread->pointers_to_send_back) {
			TM_LOG_ERROR("Failed to allocate memory for pointers to send back array.");
			return status;
		}
	}

	for(int i = 0; i < thread->clients_count; i++)
		thread->bitrate[i].bitrate_sample_count = -1;

	pipe(thread->pipe_fd);
	fcntl(thread->pipe_fd[0], F_SETFD, O_NONBLOCK);
	fcntl(thread->pipe_fd[1], F_SETFD, O_NONBLOCK);

	if (configuration.return_pointers_through_pipes) {
		pipe(thread->pipe_fd_return);
		fcntl(thread->pipe_fd_return[0], F_SETFD, O_NONBLOCK);
		fcntl(thread->pipe_fd_return[1], F_SETFD, O_NONBLOCK);
	}

	thread->loop = ev_loop_new(0);
	if (!thread->loop) {
		TM_LOG_ERROR("Failed to create loop for work thread.");
		return status;
	}

	ev_io_init(&thread->w_pipe_write, pipe_write_callback, thread->pipe_fd[1], EV_WRITE);
	ev_io_start(main_thread.loop, &thread->w_pipe_write);

	if (configuration.return_pointers_through_pipes) {
		ev_io_init(&thread->w_pipe_ret_ptrs_read, return_ptrs_read_callback, thread->pipe_fd_return[0], EV_READ);
		ev_io_start(main_thread.loop, &thread->w_pipe_ret_ptrs_read);
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
		ev_loop_destroy(thread->loop);
		if (thread->pipe_fd[0] > 0)
			close(thread->pipe_fd[0]);
		if (thread->pipe_fd[1] > 0)
			close(thread->pipe_fd[1]);
		if (thread->pipe_fd_return[0] > 0)
			close(thread->pipe_fd_return[0]);
		if (thread->pipe_fd_return[1] > 0)
			close(thread->pipe_fd_return[1]);
		tm_free(thread->pointers_to_send_back);
		tm_free(thread->bitrate);
	}
}

static void stop_main_thread_watchers()
{
	for (int i = 0; i < SIGNAL_COUNT; i++) {
		ev_signal_stop(main_thread.loop, &main_thread.w_signals[i]);
	}
	ev_timer_stop(main_thread.loop, &main_thread.w_test_time);
	ev_async_stop(main_thread.loop, &main_thread.w_low_bitrate);

	for (int i = 0; i < main_thread.work_threads_count; i ++) {
		ev_io_stop(main_thread.loop, &main_thread.work_threads[i].w_pipe_write);
		if (configuration.return_pointers_through_pipes) {
			ev_io_stop(main_thread.loop, &main_thread.work_threads[i].w_pipe_ret_ptrs_read);
		}
		ev_timer_stop(main_thread.loop, &main_thread.work_threads[i].w_next_io_start);
	}
}

static void signal_callback(EV_P_ ev_signal *w, int revents)
{
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

static void test_time_callback(EV_P_ ev_timer *w, int revents)
{
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для сигнализации главному потоку о низком битрейте.
 * Крутится на петле главного потока. Посылают рабочие потоки.
 */
static void low_bitrate_callback(EV_P_ ev_async *w, int revents)
{
	main_thread.low_bitrate_flag = 1;
	stop_main_thread_watchers();
	ev_break(loop, EVBREAK_ONE);
}

TMThreadStatus tm_threads_init_events(int count)
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

TMThreadStatus tm_threads_work_events()
{
	ev_timer_start(main_thread.loop, &main_thread.w_test_time);
	ev_run(main_thread.loop, 0);

	if (main_thread.low_bitrate_flag) {
		TM_LOG_TRACE("Low bitrate");
		return TMThreadStatus_ERROR;
	}

	return TMThreadStatus_SUCCESS;
}

TMThreadStatus tm_threads_shutdown_events()
{
	TMThreadStatus status = TMThreadStatus_SUCCESS;

	for (int i = 0; i < main_thread.work_threads_count; i++) {
		tm_thread_shutdown(&main_thread.work_threads[i]);
	}
	tm_free(main_thread.work_threads);
	ev_loop_destroy(main_thread.loop);

	return status;
}