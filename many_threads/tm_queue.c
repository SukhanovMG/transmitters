
#include "tm_queue.h"
#include "tm_read_config.h"
#include "tm_alloc.h"
#include "tm_logging.h"
#include <stdio.h>


tm_queue_ctx *tm_queue_create()
{
	tm_queue_ctx *q = tm_calloc(sizeof(tm_queue_ctx));
	if (!q)
		return q;

	if(pthread_mutex_init(&q->mutex, NULL) != 0)
	{
		tm_free(q);
		q = NULL;
		return q;
	}
	if (pthread_cond_init(&q->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&q->mutex);
		tm_free(q);
		q = NULL;
		return q;
	}

	q->finish = 0;

	return q;
}

int tm_queue_push_back(tm_queue_ctx *q, tm_block *block)
{
	//TM_LOG_DTRACE();
	tm_queue_elem_ctx *elem = NULL;

	if (!q)
		return 0;

	elem = tm_calloc(sizeof(tm_queue_elem_ctx));
	if (!elem)
		return 0;

	elem->block = tm_block_transfer_block(block);

	pthread_mutex_lock(&q->mutex);
	if (q->count == 0)
	{
		q->tail = elem;
		q->head = q->tail;
	}
	else
	{
		q->tail->next = elem;
		q->tail = elem;
	}
	q->count++;
	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);

	return 1;
}

tm_block *tm_queue_pop_front(tm_queue_ctx *q)
{
	//TM_LOG_DTRACE();
	tm_queue_elem_ctx *elem = NULL;
	tm_block *block = NULL;

	if (!q)
		return NULL;

	pthread_mutex_lock(&q->mutex);
	while (q->count == 0)
	{
		pthread_cond_wait(&q->cond, &q->mutex);
		if (q->finish)
		{
			pthread_mutex_unlock(&q->mutex);
			return NULL;
		}
	}

	elem = q->head;
	if (q->count == 1)
	{
		q->head = NULL;
		q->tail = NULL;
	}
	else
		q->head = q->head->next;

	q->count--;
	pthread_mutex_unlock(&q->mutex);

	block = elem->block;
	tm_free(elem);
	return block;
}


int tm_queue_destroy(tm_queue_ctx *q)
{
	if(!q)
		return 0;

	while (q->count > 0)
	{
		tm_refcount_release((void *)tm_queue_pop_front(q));
	}

	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);

	tm_free(q);

	return 1;
}

