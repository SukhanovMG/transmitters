#include "tm_queue_simple.h"
#include "tm_configuration.h"
#include "tm_mempool.h"
#include <uthash/utlist.h>

typedef struct _queue_elem_simple {
	struct _queue_elem_simple *next;
	struct _queue_elem_simple *prev;
	client_block_t client_block;
} queue_elem_simple;

typedef struct _queue_simple {
	queue_elem_simple *head;
	tm_mempool *pool;
} queue_simple;

void tm_queue_destroy_simple(void *_q)
{
	queue_simple *q = (queue_simple *)_q;
	if (q) {
		while (q->head) {
			tm_queue_pop_front_simple(q);
		}
		if (q->pool)
			tm_mempool_delete(q->pool);
		tm_free(q);
	}
}

void *tm_queue_create_simple()
{
	queue_simple *q = tm_calloc(sizeof(queue_simple));
	if (!q){
		return NULL;
	}
	if (configuration.queue_type == kTmQueueSimpleMempool) {
		q->pool = tm_mempool_new(sizeof(queue_elem_simple), 100000, 1);
		if (!q->pool) {
			tm_free(q);
			return NULL;
		}
	}
	return (void *) q;
}

int tm_queue_push_back_simple(void *_q, client_block_t *client_block)
{
	queue_simple *q = (queue_simple *)_q;
	queue_elem_simple *elem = NULL;

	if (!q || !client_block)
		return 0;

	if (!q->pool)
		elem = tm_calloc(sizeof(queue_elem_simple));
	else
		elem = tm_mempool_get(q->pool);

	if (!elem)
		return 0;

	elem->client_block = *client_block;

	DL_APPEND(q->head, elem);

	return 1;
}

client_block_t tm_queue_pop_front_simple(void *_q)
{
	queue_simple *q = (queue_simple *)_q;
	queue_elem_simple *elem = NULL;
	client_block_t client_block = { NULL, 0 };
	if (q && q->head) {
		elem = q->head;
		DL_DELETE(q->head, elem);
		client_block = elem->client_block;
		if (!q->pool)
			tm_free(elem);
		else
			tm_mempool_return(q->pool, elem);
	}

	return client_block;
}

int tm_queue_is_empty_simple(void *_q)
{
	queue_simple *q = (queue_simple *)_q;
	return q && !q->head;
}

