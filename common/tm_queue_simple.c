#include "tm_queue_simple.h"
#include "tm_configuration.h"
#include <uthash/utlist.h>

typedef struct _queue_elem_simple {
	struct _queue_elem_simple *next;
	struct _queue_elem_simple *prev;
	client_block_t client_block;
} queue_elem_simple;

typedef struct _queue_simple {
	queue_elem_simple *head;
	tm_allocator queue_elem_allocator;
} queue_simple;

void tm_queue_destroy_simple(void *_q)
{
	queue_simple *q = (queue_simple *)_q;
	if (q) {
		while (q->head) {
			tm_queue_pop_front_simple(q);
		}
		tm_free(q);
	}
}

void *tm_queue_create_simple(tm_allocator allocator)
{
	queue_simple *q = tm_calloc(sizeof(queue_simple));
	if (!q){
		return q;
	}

	q->queue_elem_allocator = allocator;
	return (void *) q;
}

int tm_queue_push_back_simple(void *_q, client_block_t *client_block)
{
	queue_simple *q = (queue_simple *)_q;
	queue_elem_simple *elem = NULL;

	if (!q || !client_block)
		return 0;

	elem = tm_calloc_custom(sizeof(queue_elem_simple), &q->queue_elem_allocator);
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
		tm_free_custom(elem, &q->queue_elem_allocator);
	}

	return client_block;
}

int tm_queue_is_empty_simple(void *_q)
{
	queue_simple *q = (queue_simple *)_q;
	return q && !q->head;
}

