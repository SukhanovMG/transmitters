#ifndef TM_QUEUE_LOCKLESS2_H
#define TM_QUEUE_LOCKLESS2_H

#include "tm_block.h"
#include "tm_alloc.h"

void tm_queue_destroy_lockless2(void *_q);
void *tm_queue_create_lockless2(tm_allocator allocator);
int tm_queue_push_back_lockless2(void *_q, client_block_t *client_block);
client_block_t tm_queue_pop_front_lockless2(void *_q);
int tm_queue_is_empty_lockless2(void *_q);

#endif /*TM_QUEUE_LOCKLESS2_H*/