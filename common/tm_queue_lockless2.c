#include "tm_queue_lockless2.h"
#include "tm_configuration.h"
#include "tm_logging.h"

#include "lqueue.h"
#include "tm_alloc.h"

typedef struct {
	lqueue *q;
	tm_allocator queue_elem_allocator;
} queue_lockless;

void tm_queue_destroy_lockless2(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	if (q) {
		lqueue_delete(q->q);
		tm_free(q);
	}
}

void *tm_queue_create_lockless2(tm_allocator allocator)
{
	queue_lockless *q = tm_calloc(sizeof(queue_lockless));
	if (q){
		q->q = lqueue_new();
		if (q->q) {
			q->queue_elem_allocator = allocator;
		} else {
			tm_free(q);
			q = NULL;
		}
	}

	return (void *) q;
}

int tm_queue_push_back_lockless2(void *_q, client_block_t *client_block)
{
	int result = 0;
	queue_lockless *q = (queue_lockless *)_q;
	if (q && client_block) {
		client_block_t *b = tm_alloc_custom(sizeof(client_block_t), &q->queue_elem_allocator);
		if (b) {
			b->block = client_block->block;
			b->client_id = client_block->client_id;

			lqueue_push_back(q->q, (void*) b);
			result = 1;
		}
	}
	return result;
}

client_block_t tm_queue_pop_front_lockless2(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	client_block_t client_block = { NULL, 0 };
	if (q) {
		client_block_t *b = NULL;
		lqueue_pop_front(q->q, (void**)&b);
		if (b) {
			client_block = *b;
			tm_free_custom(b, &q->queue_elem_allocator);
		}
	}
	return client_block;
}

int tm_queue_is_empty_lockless2(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	return ((lqueue_elem*)q->q->head.ptr)->next.ptr == NULL;
}