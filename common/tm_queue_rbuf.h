#ifndef TM_QUEUE_RBUF_H
#define TM_QUEUE_RBUF_H

#include "tm_block.h"
#include "tm_alloc.h"

void tm_queue_destroy_rbuf(void *_q);
void *tm_queue_create_rbuf(tm_allocator allocator);
int tm_queue_push_back_rbuf(void *_q, client_block_t *client_block);
client_block_t tm_queue_pop_front_rbuf(void *_q);
int tm_queue_is_empty_rbuf(void *_q);

#endif /*TM_QUEUE_RBUF_H*/