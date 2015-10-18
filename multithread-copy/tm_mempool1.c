#include "tm_mempool.h"

#include "tm_alloc.h"
#include "uthash/utlist.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct _mempool_elem {
	struct _mempool_elem *next;
	struct _mempool_elem *prev;

	char data[];
} mempool_elem;

typedef struct _mempool {
	pthread_mutex_t mutex;

	unsigned int elem_size;
	unsigned int count;
	unsigned int free;

	mempool_elem *elements;
} mempool;

void tm_mempool_delete(tm_mempool *pool)
{
	mempool *_pool = (mempool*)pool;
	if (pool) {
		mempool_elem *e, *tmp;
		DL_FOREACH_SAFE(_pool->elements, e, tmp)
		{
			DL_DELETE(_pool->elements, e);
			free(e);
		}

		pthread_mutex_destroy(&_pool->mutex);

		free(_pool);
	}
}

tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count)
{
	tm_mempool *_pool = NULL;
	mempool *pool = NULL;

	pool = tm_calloc(sizeof(mempool));
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
			mempool_elem *e = tm_alloc(sizeof(mempool_elem) + elem_size);
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

	_pool = (tm_mempool*)pool;
	return _pool;
}

void *tm_mempool_get(tm_mempool *pool)
{
	void *data = NULL;
	mempool *_pool = (mempool*)pool;

	if (pool) {
		pthread_mutex_lock(&_pool->mutex);

		if (_pool->free == 0) {
			for (int i = 0; i < _pool->count; i++) {
				mempool_elem *e = tm_alloc(sizeof(mempool_elem) + _pool->elem_size);
				if (e) {
					DL_APPEND(_pool->elements, e);
					_pool->free++;
				}
			}
			_pool->count += _pool->free;
		}

		if (_pool->free > 0)
		{
			mempool_elem *e = _pool->elements;
			DL_DELETE(_pool->elements, e);
			data = e->data;
			_pool->free--;
		}
		pthread_mutex_unlock(&_pool->mutex);
	}

	return data;
}

void tm_mempool_return(tm_mempool *pool, void *data)
{
	mempool *_pool = (mempool*)pool;
	if (pool && data)
	{
		mempool_elem *e = data - sizeof(mempool_elem);
		pthread_mutex_lock(&_pool->mutex);
		DL_APPEND(_pool->elements, e);
		_pool->free++;
		pthread_mutex_unlock(&_pool->mutex);
	}
}
