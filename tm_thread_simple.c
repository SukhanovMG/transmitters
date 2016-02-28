#include "tm_thread_simple.h"

#include "tm_alloc.h"
#include "tm_queue.h"
#include "tm_block.h"
#include "tm_logging.h"
#include "tm_configuration.h"
#include "tm_time.h"

#include <pthread.h>
#include <signal.h>
#include <math.h>
#include <tm_queue.h>

typedef struct _tm_thread_t {
	pthread_t thread;
	tm_queue_ctx *queue;
	pthread_mutex_t q_mutex;
	pthread_cond_t q_cond;
	volatile int shutdown;
	tm_time_bitrate *bitrate_ctx;
	int clients_count;
} tm_thread_t;

typedef struct _tm_threads_t {
	tm_thread_t *threads;
	int threads_num;
	double start_time;
} tm_threads_t;

#define NEED_TO_SHUTDOWN_THREAD(thread_ctx) (__sync_add_and_fetch(&thread_ctx->shutdown, 0))
#define MARK_THREAD_FOR_SHUTDOWN(thread_ctx) \
{ \
	__sync_add_and_fetch(&thread_ctx->shutdown, 1); \
	pthread_cond_signal(&thread_ctx->q_cond); \
}

static int tm_shutdown_flag = 0;
static int tm_low_bitrate_flag = 0;
static tm_threads_t work_threads;

static int tm_thread_pop_from_queue(tm_thread_t *thread_ctx, client_block_t *array, int count)
{
	int result = 0;
	if (thread_ctx) {
		pthread_mutex_lock(&thread_ctx->q_mutex);
		while (tm_queue_is_empty(thread_ctx->queue)) {
			pthread_cond_wait(&thread_ctx->q_cond, &thread_ctx->q_mutex);
			if (NEED_TO_SHUTDOWN_THREAD(thread_ctx)) {
				pthread_mutex_unlock(&thread_ctx->q_mutex);
				return result;
			}
		}
		result = tm_queue_pop_front(thread_ctx->queue, array, count);
		pthread_mutex_unlock(&thread_ctx->q_mutex);
	}
	return result;
}

static void tm_thread_push_to_queue(tm_thread_t *thread_ctx, client_block_t *array, int count)
{
	pthread_mutex_lock(&thread_ctx->q_mutex);
	tm_queue_push_back(thread_ctx->queue, array, count);
	pthread_cond_signal(&thread_ctx->q_cond);
	pthread_mutex_unlock(&thread_ctx->q_mutex);
}

static void tm_thread_function(void* thread)
{
	tm_thread_t *thread_ctx = (tm_thread_t*)thread;
	client_block_t *client_block_array = tm_calloc(128 * sizeof(client_block_t));

	while(!NEED_TO_SHUTDOWN_THREAD(thread_ctx)) {
		if (tm_thread_pop_from_queue(thread_ctx, client_block_array, 128)) {
			double cur_time = tm_time_get_current_ntime();
			for (int i = 0; i < 128; i++) {
				if (client_block_array[i].block == NULL)
					break; // break for
				tm_block_dispose_block(client_block_array[i].block);
				if (sample_bitrate(&thread_ctx->bitrate_ctx[client_block_array[i].client_id], cur_time))
				{
					if (thread_ctx->bitrate_ctx[client_block_array[i].client_id].bitrate <= (double) configuration.bitrate - configuration.bitrate_diff)
					{
						tm_low_bitrate_flag = 1;
						goto thread_shutdown; // break for and while
					}
				}
			}
		}
	}
thread_shutdown:
	tm_free(client_block_array);
}

static TMThreadStatus tm_thread_thread_create(tm_thread_t *thread)
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	int pthread_create_ret;

	if (!thread)
		return status;

	thread->queue = tm_queue_create();
	if(!thread->queue)
		return status;

	thread->shutdown = 0;

	if(pthread_mutex_init(&thread->q_mutex, NULL) != 0)
		return status;

	if (pthread_cond_init(&thread->q_cond, NULL) != 0)
		return status;

	thread->bitrate_ctx = (tm_time_bitrate*)tm_calloc(thread->clients_count * sizeof(tm_time_bitrate));
	if(!thread->bitrate_ctx)
		return status;
	for(int i = 0; i < thread->clients_count; i++)
		thread->bitrate_ctx[i].bitrate_sample_count = -1;

	pthread_create_ret = pthread_create(&thread->thread, NULL, (void*(*)(void *)) tm_thread_function, thread);
	if(pthread_create_ret != 0)
	{
		tm_queue_destroy(thread->queue);
		return status;
	}

	status = TMThreadStatus_SUCCESS;
	return status;
}

static TMThreadStatus tm_thread_thread_shutdown(tm_thread_t *thread)
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	void *thread_status;

	if (!thread)
		return status;

	MARK_THREAD_FOR_SHUTDOWN(thread);

	pthread_join(thread->thread, &thread_status);
	pthread_cond_destroy(&thread->q_cond);
	pthread_mutex_destroy(&thread->q_mutex);
	tm_free(thread->bitrate_ctx);
	tm_queue_destroy(thread->queue);

	status = TMThreadStatus_SUCCESS;
	return status;
}

/**
 * Обработчик сигналов.
 */
void tm_signal_handler(int signum)
{
	TM_LOG_TRACE("transcoder_signal_handler");
	tm_shutdown_flag = 1;
}

TMThreadStatus tm_threads_init_simple(int count)
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if(count < 1)
		return status;

	work_threads.threads_num = 0;
	work_threads.threads = tm_calloc(count * sizeof(tm_thread_t));

	if(!work_threads.threads)
		return status;

	for(int i = 0; i < count; i++)
	{
		work_threads.threads[i].clients_count = configuration.clients_count / count;
	}
	for(int i = 0; i < configuration.clients_count % count; i++)
	{
		work_threads.threads[i].clients_count++;
	}

	for(int i = 0; i < count; i++)
	{
		if (tm_thread_thread_create(&work_threads.threads[i]) != TMThreadStatus_SUCCESS)
			return TMThreadStatus_ERROR;

		work_threads.threads_num++; // реальное число созданных потоков
	}

	work_threads.start_time = 0;

	signal(SIGINT, tm_signal_handler);
	signal(SIGTERM, tm_signal_handler);
	signal(SIGQUIT, tm_signal_handler);
	signal(SIGTSTP, tm_signal_handler);
	signal(SIGPWR, tm_signal_handler);

	return TMThreadStatus_SUCCESS;

}

TMThreadStatus tm_threads_shutdown_simple()
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	while (work_threads.threads_num > 0)
	{
		tm_thread_thread_shutdown(&work_threads.threads[work_threads.threads_num - 1]);
		work_threads.threads_num--;
	}

	tm_free(work_threads.threads);

	status = TMThreadStatus_SUCCESS;
	return status;

}

TMThreadStatus tm_threads_work_simple()
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	client_block_t *client_block_array = NULL;
	int client_block_array_size = work_threads.threads[0].clients_count;
	tm_block *block = NULL;
	double current_time = 0;

	client_block_array = tm_calloc(client_block_array_size * sizeof(client_block_t));
	if (!client_block_array)
		return status;

	work_threads.start_time = tm_time_get_current_ntime();

	while(!tm_shutdown_flag && !tm_low_bitrate_flag)
	{
		double time_after_work = 0.0;
		useconds_t diff = 0;
		useconds_t corrected_sleep_time = 0;

		current_time = tm_time_get_current_ntime();
		if (current_time - work_threads.start_time >= (double) configuration.test_time)
			break;
		block = tm_block_create();
		for (int i = 0; i < work_threads.threads_num; i++)
		{
			for (size_t j = 0; j < work_threads.threads[i].clients_count; j++)
			{
				client_block_array[j].block = tm_block_transfer_block(block);
				client_block_array[j].client_id = j;
			}
			tm_thread_push_to_queue(&work_threads.threads[i], client_block_array, work_threads.threads[i].clients_count);
		}
		tm_block_dispose_block(block);
		time_after_work = tm_time_get_current_ntime();
		diff = (useconds_t)((time_after_work - current_time) * 1000000);
		while (diff > configuration.sleep_time)
			diff -= configuration.sleep_time;

		corrected_sleep_time = configuration.sleep_time - diff;
		usleep(corrected_sleep_time);
	}

	if (tm_low_bitrate_flag)
		TM_LOG_TRACE("Low bitrate.");
	else
		status = TMThreadStatus_SUCCESS;

	tm_free(client_block_array);
	return status;
}
