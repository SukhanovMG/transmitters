#ifndef TM_QUEUE_SIMPLE_H
#define TM_QUEUE_SIMPLE_H

#include "tm_block.h"
#include "tm_alloc.h"

void tm_queue_destroy_simple(void *_q);
void *tm_queue_create_simple(tm_allocator allocator);
int tm_queue_push_back_simple(void *_q, client_block_t *client_block);
client_block_t tm_queue_pop_front_simple(void *_q);
int tm_queue_is_empty_simple(void *_q);

#endif /*TM_QUEUE_SIMPLE_H*/