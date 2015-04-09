#include "tm_thread.h"
#include "tm_queue.h"
#include <pthread.h>

typedef struct _tm_thread_t {
	pthread_t thread;
	tm_queue_t *queue;
	int shutdown;
} tm_thread_t;

typedef struct _tm_threads_t {
	tm_thread_t *threads;
	int threads_num;
} tm_threads_t;

tm_threads_t work_threads;

static void tm_thread_function(void* queue)
{

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
	pthread_cond_signal(&thread->queue->cond);
	pthread_mutex_unlock(&thread->queue->mutex);

	pthread_join(thread->thread, &thread_status);
	tm_queue_destroy(thread->queue);

	status = TMThreadStatus_SUCCESS;
	return status;
}

TMThreadStatus tm_threads_init(int count)
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if(count < 1)
		return status;

	work_threads.threads_num = 0;
	work_threads.threads = tm_alloc(count * sizeof(tm_thread_t));

	if(!work_threads.threads)
		return status;

	for(int i = 0; i < count; i++)
	{
		if (tm_thread_thread_create(&work_threads.threads[i]) != TMThreadStatus_SUCCESS)
			return TMThreadStatus_ERROR;

		work_threads.threads_num++; // реальное число созданных потоков
	}

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

	status = TMThreadStatus_SUCCESS;
	return status;

}
