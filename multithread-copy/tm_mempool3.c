#include "tm_mempool.h"

#include "tm_alloc.h"
#include "uthash/utlist.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct _elem {
	struct _elem *prev;
	struct _elem *next;
	unsigned int used_blocks;
	unsigned int current_block;
	char data[];
} elem;

typedef struct _block {
	elem* element;
	char data[];
} block;

typedef struct _mempool {
	pthread_mutex_t mutex;
	elem *available_elements;
	elem *current_element;
	unsigned int block_size;
	unsigned int blocks_in_elem;
} mempool;

static elem *mempool_element_new(mempool *pool)
{
	elem *element = NULL;
	if (pool) {
		element = tm_calloc(sizeof(element) + (sizeof(block) + pool->block_size) * pool->blocks_in_elem);
		if (element) {
			for (int i = 0; i < pool->blocks_in_elem; i++) {
				block *b = (block*)(((void*)element->data) + i * (sizeof(block) + pool->block_size));
				b->element = element;
			}
		}
	}
	return element;
}

tm_mempool *tm_mempool_new(unsigned int elem_size, unsigned int count)
{
	tm_mempool *_pool = NULL;
	mempool *pool = NULL;

	pool = tm_calloc(sizeof(mempool));
	if (pool) {
		if (pthread_mutex_init(&pool->mutex, NULL) == 0) {
			elem *element = NULL;
			pool->block_size = elem_size;
			pool->blocks_in_elem = count;
			element = mempool_element_new(pool);
			if (element) {
				pool->available_elements = NULL;
				pool->current_element = element;
			} else {
				pthread_mutex_destroy(&pool->mutex);
				tm_free(pool);
				pool = NULL;
			}
		} else {
			tm_free(pool);
			pool = NULL;
		}
	}

	_pool = (tm_mempool*) pool;
	return _pool;
}

void tm_mempool_delete(tm_mempool *pool)
{
	mempool *_pool = (mempool*)pool;

	if (pool) {
		// Освобождение текущего элемента пула
		if (_pool->current_element) {
			tm_free(_pool->current_element);
		}
		/*
		 * Освобождение всех остальных элементов пула.
		 * К моменту вызова этой функции все элементы пула должны вернуться в
		 * список доступных.
		 */
		elem *e, *tmp;
		DL_FOREACH_SAFE(_pool->available_elements, e, tmp) {
			DL_DELETE(_pool->available_elements, e);
			tm_free(e);
		}

		pthread_mutex_destroy(&_pool->mutex);

		// Освобождение контекста пула
		tm_free(_pool);
	}
}

static int mempool_element_has_available_blocks(mempool *pool, elem* element)
{
	int res = 0;

	if (pool && element){
		if (element->current_block < pool->blocks_in_elem)
			res = 1;
	}

	return res;
}

static int mempool_update_current_element(mempool *pool)
{
	int res = 0;

	if (pool) {
		if (!mempool_element_has_available_blocks(pool, pool->current_element)) {
			elem *element = NULL;
			if (pool->available_elements) {
				element = pool->available_elements;
				DL_DELETE(pool->available_elements, element);
			} else {
				element = mempool_element_new(pool);
			}

			if (element) {
				pool->current_element = element;
				res = 1;
			}
		} else {
			res = 1;
		}
	}

	return res;
}

void *tm_mempool_get(tm_mempool *pool)
{
	mempool *_pool = (mempool*) pool;
	void *ptr = NULL;

	pthread_mutex_lock(&_pool->mutex);
	if (mempool_update_current_element(_pool)) {
		block *b = (block*)(((void*)_pool->current_element->data) + _pool->current_element->current_block * (sizeof(block) + _pool->block_size));
		ptr = b->data;
		_pool->current_element->used_blocks++;
		_pool->current_element->current_block++;
	}
	pthread_mutex_unlock(&_pool->mutex);

	return ptr;
}

void tm_mempool_return(tm_mempool *pool, void *data)
{
	mempool *_pool = (mempool*) pool;
	pthread_mutex_lock(&_pool->mutex);
	if (pool && data) {
		block *b = (block*)(data - sizeof(block));
		elem *element = b->element;
		if (element) {
			element->used_blocks--;
			if (element->used_blocks == 0) {
				if (!mempool_element_has_available_blocks(pool, element)) {
					element->current_block = 0;
				}

				if (element != _pool->current_element) {
					DL_APPEND(_pool->available_elements, element);
				}
			}
		}
	}
	pthread_mutex_unlock(&_pool->mutex);
}
