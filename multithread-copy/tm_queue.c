
#include "tm_queue.h"
#include "tm_read_config.h"
#include "tm_alloc.h"
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

	return q;
}

int tm_queue_push_back(tm_queue_ctx *q)
{

	tm_queue_elem_ctx *elem = NULL;

	if (!q)
		return 0;

	elem = tm_calloc(sizeof(tm_queue_elem_ctx));
	if (!elem)
		return 0;

	elem->block = tm_calloc(configuration.block_size);

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
	pthread_mutex_unlock(&q->mutex);

	return 1;
}

void *tm_queue_pop_front(tm_queue_ctx *q)
{
	tm_queue_elem_ctx *elem = NULL;
	void *block = NULL;

	if (!q)
		return NULL;

	pthread_mutex_lock(&q->mutex);
	while (q->count == 0)
		pthread_cond_wait(&q->cond, &q->mutex);

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
		tm_free(tm_queue_pop_front(q));
	}

	tm_free(q);

	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);

	return 1;
}

