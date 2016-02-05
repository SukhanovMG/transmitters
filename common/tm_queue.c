#include "tm_queue.h"
#include "tm_alloc.h"
#include <uthash/utlist.h>

client_block_t tm_queue_pop_front_inner(tm_queue_ctx *q);
int tm_queue_push_back_inner(tm_queue_ctx *q, client_block_t *block_queue_elem);

void tm_queue_destroy(tm_queue_ctx *q)
{
	if (q) {
		while (q->head) {
			tm_queue_pop_front_inner(q);
		}
		pthread_cond_destroy(&q->cond);
		pthread_mutex_destroy(&q->mutex);
		tm_free(q);
	}
}

tm_queue_ctx *tm_queue_create()
{
	tm_queue_ctx *q = tm_calloc(sizeof(tm_queue_ctx));
	if (!q){
		return q;
	}

	if(pthread_mutex_init(&q->mutex, NULL) != 0)
	{
		tm_queue_destroy(q);
		return NULL;
	}
	if (pthread_cond_init(&q->cond, NULL) != 0)
	{
		tm_queue_destroy(q);
		return NULL;
	}

	return q;
}

int tm_queue_push_back_inner(tm_queue_ctx *q, client_block_t *client_block)
{
	tm_queue_elem_ctx *elem = NULL;

	if (!q || !client_block)
		return 0;

	elem = tm_calloc(sizeof(tm_queue_elem_ctx));
	if (!elem)
		return 0;

	elem->client_block = *client_block;

	DL_APPEND(q->head, elem);

	return 1;
}

client_block_t tm_queue_pop_front_inner(tm_queue_ctx *q)
{
	tm_queue_elem_ctx *elem = NULL;
	client_block_t client_block = { NULL, 0 };
	if (q && q->head) {
		elem = q->head;
		DL_DELETE(q->head, elem);
		client_block = elem->client_block;
		tm_free(elem);
	}

	return client_block;
}

int tm_queue_push_back(tm_queue_ctx *q, client_block_t *client_block_array, int count)
{
	int result = 0;

	if (!q || !client_block_array || count <= 0)
		return result;

	pthread_mutex_lock(&q->mutex);
	for (int i = 0; i < count; i++) {
		result = tm_queue_push_back_inner(q, &client_block_array[i]);
		if (result != 1)
			break;
	}
	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);

	return result;
}

int tm_queue_pop_front(tm_queue_ctx *q, client_block_t *client_block_array, int count)
{
	int result = 0;

	if (!q || !client_block_array || count <= 0)
		return result;

	pthread_mutex_lock(&q->mutex);
	while (!q->head) {
		pthread_cond_wait(&q->cond, &q->mutex);
		if (q->break_flag)
		{
			pthread_mutex_unlock(&q->mutex);
			return result;
		}
	}

	result = 1;
	for (int i = 0; i < count; i++) {
		client_block_array[i] = tm_queue_pop_front_inner(q);
		if (client_block_array[i].block == NULL)
			break;
	}
	pthread_mutex_unlock(&q->mutex);
	return result;
}

