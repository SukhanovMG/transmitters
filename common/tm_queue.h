
#ifndef TM_QUEUE_H
#define TM_QUEUE_H

#include <pthread.h>
#include "tm_block.h"
#include "tm_alloc.h"

typedef struct {
	tm_block *block;  // указатель на блок
	size_t client_id; // id клиента, для которого предназначен блок
} client_block_t;

typedef struct _tm_queue_elem_ctx {
	struct _tm_queue_elem_ctx *next;
	struct _tm_queue_elem_ctx *prev;
	client_block_t client_block;
} tm_queue_elem_ctx;

typedef struct _tm_queue_ctx {
	tm_queue_elem_ctx *head;
	tm_allocator queue_elem_allocator;
} tm_queue_ctx;

tm_queue_ctx *tm_queue_create();
void tm_queue_destroy(tm_queue_ctx *q);

int tm_queue_push_back(tm_queue_ctx *q, client_block_t *client_block_array, int count);
int tm_queue_pop_front(tm_queue_ctx *q, client_block_t *client_block_array, int count);

int tm_queue_is_empty(tm_queue_ctx *q);

#endif /* TM_QUEUE_H */
