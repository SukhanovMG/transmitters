#include "tm_mempool.h"

#include "tm_alloc.h"

#include <stdlib.h>

#include "uthash/utlist.h"

#include <pthread.h>

typedef struct _mempool_elem {
	struct _mempool_elem *next;
	struct _mempool_elem *prev;

	char data[];
} mempool_elem;

typedef struct _mempool {
	pthread_mutex_t mutex;
	int thread_safe;

	unsigned int elem_size;
	unsigned int count;
	unsigned int free;

	mempool_elem *initial_elements;
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
		}

		DL_FOREACH_SAFE(_pool->initial_elements, e, tmp){
			DL_DELETE(_pool->initial_elements, e);
			free(e);
		}

		if (_pool->thread_safe)
			pthread_mutex_destroy(&_pool->mutex);

		free(_pool);
	}
}

static int tm_mempool_grow(mempool* pool, unsigned int count)
{
	int ret = 0;
	if (count == 0) {
		ret = 1;
	} else {
		mempool_elem *initial_element = tm_alloc(sizeof(mempool_elem) + (sizeof(mempool_elem) + pool->elem_size) * count);
		if (initial_element) {
			DL_APPEND(pool->initial_elements, initial_element);
			void *e = (void*)initial_element->data;
			for (int i = 0; i < count; i++)
			{
				DL_APPEND(pool->elements, (mempool_elem*)e);
				e += sizeof(mempool_elem) + pool->elem_size;
				pool->free++;
				pool->count++;
			}
			ret = 1;
		}
	}

	return ret;
}

tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count, int thread_safe)
{
	tm_mempool *_pool = NULL;
	mempool *pool = NULL;

	pool = tm_calloc(sizeof(mempool));
	if (pool) {

		pool->thread_safe = thread_safe;

		if(pool->thread_safe && pthread_mutex_init(&pool->mutex, NULL) != 0)
		{
			free(pool);
			return NULL;
		}

		pool->elem_size = elem_size;

		if (!tm_mempool_grow(pool, count)) {
			if (pool->thread_safe)
				pthread_mutex_destroy(&pool->mutex);
			free(pool);
			return NULL;
		}
	}

	_pool = (void*) pool;
	return _pool;
}

void *tm_mempool_get(tm_mempool *pool)
{
	void *data = NULL;
	mempool *_pool = (mempool*) pool;

	if (pool) {

		if (_pool->thread_safe)
			pthread_mutex_lock(&_pool->mutex);

		if (_pool->free == 0) {
			tm_mempool_grow(_pool, _pool->count);
		}

		if (_pool->free > 0)
		{
			mempool_elem *e = _pool->elements;
			DL_DELETE(_pool->elements, e);
			data = e->data;
			_pool->free--;
		}

		if (_pool->thread_safe)
			pthread_mutex_unlock(&_pool->mutex);
	}

	return data;
}

void tm_mempool_return(tm_mempool *pool, void *data)
{
	mempool *_pool = (mempool*) pool;
	if (pool && data)
	{
		mempool_elem *e = data - sizeof(mempool_elem);
		if (_pool->thread_safe)
			pthread_mutex_lock(&_pool->mutex);
		DL_APPEND(_pool->elements, e);
		_pool->free++;
		if (_pool->thread_safe)
			pthread_mutex_unlock(&_pool->mutex);
	}
}
