
#ifndef TM_QUEUE_H
#define TM_QUEUE_H

#include <pthread.h>
#include "tm_block.h"

typedef struct _tm_queue_elem_ctx {
	struct _tm_queue_elem_ctx *next;
	tm_block *block;
} tm_queue_elem_ctx;

typedef struct _tm_queue_ctx {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	tm_queue_elem_ctx *head;
	tm_queue_elem_ctx *tail;
	unsigned long long int count;
	int finish;
} tm_queue_ctx;

tm_queue_ctx *tm_queue_create();
int tm_queue_push_back(tm_queue_ctx *q, tm_block *block);
tm_block *tm_queue_pop_front(tm_queue_ctx *q);
int tm_queue_destroy(tm_queue_ctx *q);

#endif /* TM_QUEUE_H */
