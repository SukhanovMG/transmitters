#ifndef TM_MEMPOOL_H
#define TM_MEMPOOL_H

typedef void tm_mempool;

void tm_mempool_delete(tm_mempool *pool);
tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count, int thread_safe);
void *tm_mempool_get(tm_mempool *pool);
void tm_mempool_return(tm_mempool *pool, void *data);

#endif
