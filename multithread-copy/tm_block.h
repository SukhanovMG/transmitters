#ifndef TM_BLOCK_H
#define TM_BLOCK_H

#include "tm_refcount.h"

typedef struct _tm_block {
	tm_refcount refcount;
	void *block;
} tm_block;

void tm_block_destroy(tm_block *block);
void tm_block_destructor(void *obj);
tm_block *tm_block_create();

#endif /* TM_BLOCK_H */
