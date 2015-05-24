#include "tm_thread.h"
#include "tm_queue.h"
#include "tm_block.h"
#include "tm_logging.h"
#include "tm_read_config.h"
#include "tm_time.h"
#include <pthread.h>

#include <signal.h>

typedef struct _tm_thread_t {
	pthread_t thread;
	tm_queue_ctx *queue;
	int shutdown;
	tm_time_bitrate bitrate_ctx;
} tm_thread_t;

typedef struct _tm_threads_t {
	tm_thread_t *threads;
	int threads_num;
	double start_time;
} tm_threads_t;


static int tm_shutdown_flag = 0;
static int tm_low_bitrate_flag = 0;
static tm_threads_t work_threads;

static void tm_thread_function(void* thread)
{
	tm_thread_t *thread_ctx = (tm_thread_t*)thread;
	tm_block *block = NULL;

	while(!thread_ctx->shutdown)
	{
		block = tm_queue_pop_front(thread_ctx->queue);
		if (block)
		{
			tm_refcount_release((void*)block);
			if (tm_time_sample_bitrate(&thread_ctx->bitrate_ctx))
			{
				//TM_LOG_TRACE("thread %lu bitrate %lf", thread_ctx->thread, thread_ctx->bitrate_ctx.bitrate);
				if (thread_ctx->bitrate_ctx.bitrate <= (double) configuration.bitrate - configuration.bitrate_diff)
					tm_low_bitrate_flag = 1;
			}
		}
	}
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

	thread->bitrate_ctx.bitrate_sample_count = -1;

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

	pthread_mutex_lock(&thread->queue->mutex);
	thread->shutdown = 1;
	thread->queue->finish = 1;
	pthread_cond_signal(&thread->queue->cond);
	pthread_mutex_unlock(&thread->queue->mutex);

	pthread_join(thread->thread, &thread_status);
	tm_queue_destroy(thread->queue);

	status = TMThreadStatus_SUCCESS;
	return status;
}

/**
 * Обработчик сигналов.
 */
void tm_signal_handler()
{
	TM_LOG_TRACE("transcoder_signal_handler");
	tm_shutdown_flag = 1;
}

TMThreadStatus tm_threads_init(int count)
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

TMThreadStatus tm_threads_shutdown()
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

TMThreadStatus tm_threads_work()
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	tm_block *block = NULL;
	double current_time = 0;

	work_threads.start_time = tm_time_get_current_ntime();

	while(!tm_shutdown_flag && !tm_low_bitrate_flag)
	{
		//TM_LOG_TRACE("tm_threads_work iteration");
		current_time = tm_time_get_current_ntime();
		if (current_time - work_threads.start_time >= (double) configuration.test_time)
			break;
		block = tm_block_create();
		for (int i = 0; i < work_threads.threads_num; i++)
			tm_queue_push_back(work_threads.threads[i].queue, block);
		tm_refcount_release((void*)block);
		nanosleep(&configuration.sleep_time, NULL);
	}

	if (tm_low_bitrate_flag)
		TM_LOG_TRACE("Low bitrate.");
	else
		status = TMThreadStatus_SUCCESS;
	return status;
}
