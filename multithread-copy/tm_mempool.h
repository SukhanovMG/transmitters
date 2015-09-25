#ifndef TM_MEMPOOL_H
#define TM_MEMPOOL_H

#include <pthread.h>

typedef struct _tm_mempool_elem {
	struct _tm_mempool_elem *next;
	struct _tm_mempool_elem *prev;

	char data[];
} tm_mempool_elem;

typedef struct _tm_mempool {
	pthread_mutex_t mutex;

	unsigned int elem_size;
	unsigned int count;
	unsigned int free;

	tm_mempool_elem *elements;
} tm_mempool;

void tm_mempool_delete(tm_mempool *pool);
tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count);
void *tm_mempool_get(tm_mempool *pool);
void tm_mempool_return(tm_mempool *pool, void *data);

#endif
