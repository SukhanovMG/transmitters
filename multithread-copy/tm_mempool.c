#include "tm_mempool.h"

#include "tm_alloc.h"

#include <stdlib.h>

#include "uthash/utlist.h"

void tm_mempool_delete(tm_mempool *pool)
{
	if (pool) {
		tm_mempool_elem *e, *tmp;
		DL_FOREACH_SAFE(pool->elements, e, tmp)
		{
			DL_DELETE(pool->elements, e);
			free(e);
		}

		pthread_mutex_destroy(&pool->mutex);

		free(pool);
	}
}

tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count)
{
	tm_mempool *pool = NULL;

	pool = tm_calloc(sizeof(tm_mempool));
	if (pool) {

		if(pthread_mutex_init(&pool->mutex, NULL) != 0)
		{
			tm_free(pool);
			pool = NULL;
			return pool;
		}

		pool->elem_size = elem_size;
		pool->count = count;
		for (int i = 0; i < count; i++)
		{
			tm_mempool_elem *e = malloc(sizeof(tm_mempool_elem) + elem_size);
			if (e) {
				DL_APPEND(pool->elements, e);
				pool->free++;
			}
			else
			{
				break;
			}
		}

		if (pool->free != pool->count) {
			tm_mempool_delete(pool);
			pool = NULL;
		}
	}

	return pool;
}

void *tm_mempool_get(tm_mempool *pool)
{
	void *data = NULL;

	if (pool) {
		pthread_mutex_lock(&pool->mutex);
		if (pool->free > 0)
		{
			tm_mempool_elem *e = pool->elements;
			DL_DELETE(pool->elements, e);
			data = e->data;
			pool->free--;
		}
		pthread_mutex_unlock(&pool->mutex);
	}

	return data;
}

void tm_mempool_return(tm_mempool *pool, void *data)
{
	if (pool && data)
	{
		tm_mempool_elem *e = data - sizeof(tm_mempool_elem);
		pthread_mutex_lock(&pool->mutex);
		DL_APPEND(pool->elements, e);
		pool->free++;
		pthread_mutex_unlock(&pool->mutex);
	}
}
