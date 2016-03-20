
#ifndef TM_QUEUE_H
#define TM_QUEUE_H

#include "tm_block.h"
#include "tm_alloc.h"

typedef enum {
	kTmQueueSimple = 0,
	kTmQueueLockless,
	kTmQueueLockless2
} tm_queue_type;

typedef void tm_queue_ctx;

tm_queue_ctx *tm_queue_create(tm_queue_type type);
void tm_queue_destroy(tm_queue_ctx *_q);

int tm_queue_push_back(tm_queue_ctx *_q, client_block_t *client_block_array, int count);
int tm_queue_pop_front(tm_queue_ctx *_q, client_block_t *client_block_array, int count);

int tm_queue_is_empty(tm_queue_ctx *_q);

#endif /* TM_QUEUE_H */
