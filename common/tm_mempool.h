#ifndef TM_MEMPOOL_H
#define TM_MEMPOOL_H

#include <unistd.h>

typedef void tm_mempool;

void tm_mempool_delete(tm_mempool *pool);
tm_mempool *tm_mempool_new(size_t elem_size, size_t count, int thread_safe);
void *tm_mempool_get(tm_mempool *pool);
void tm_mempool_return(tm_mempool *pool, void *data);

#endif
