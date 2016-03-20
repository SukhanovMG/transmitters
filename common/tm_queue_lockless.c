#include "tm_queue_lockless.h"
#include "tm_configuration.h"
#include "tm_logging.h"

#include "rqueue.h"
#include "tm_alloc.h"

typedef struct {
	rqueue_t *q;
	tm_allocator queue_elem_allocator;
} queue_lockless;

void tm_queue_destroy_lockless(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	if (q) {
		rqueue_destroy(q->q);
		tm_free(q);
	}
}

void *tm_queue_create_lockless(tm_allocator allocator)
{
	queue_lockless *q = tm_calloc(sizeof(queue_lockless));
	if (q){
		q->q = rqueue_create(1024*1024, RQUEUE_MODE_BLOCKING);
		if (q->q) {
			q->queue_elem_allocator = allocator;
		} else {
			tm_free(q);
			q = NULL;
		}
	}

	return (void *) q;
}

int tm_queue_push_back_lockless(void *_q, client_block_t *client_block)
{
	int result = 0;
	queue_lockless *q = (queue_lockless *)_q;
	if (q && client_block) {
		client_block_t *b = tm_alloc_custom(sizeof(client_block_t), &q->queue_elem_allocator);
		if (b) {
			b->block = client_block->block;
			b->client_id = client_block->client_id;

			if (rqueue_write(q->q, (void*) b) == 0)
				result = 1;
		}
	}
	return result;
}

client_block_t tm_queue_pop_front_lockless(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	client_block_t client_block = { NULL, 0 };
	if (q) {
		client_block_t *b = rqueue_read(q->q);
		if (b) {
			client_block = *b;
			tm_free_custom(b, &q->queue_elem_allocator);
		}
	}
	return client_block;
}

int tm_queue_is_empty_lockless(void *_q)
{
	queue_lockless *q = (queue_lockless *)_q;
	return (rqueue_isempty(q->q));
}

