#include "tm_block.h"

#include "tm_alloc.h"
#include "tm_configuration.h"
#include "tm_logging.h"
#include "tm_mempool.h"

#include <jemalloc/jemalloc.h>

#define TM_BLOCK_DEBUG 0

#if !TM_BLOCK_DEBUG
#define TM_LOG_DTRACE(...)
#undef 	TM_LOG_STRACE
#define TM_LOG_STRACE()
#undef 	TM_LOG_TSTRACE
#define TM_LOG_TSTRACE(id)
#undef 	TM_LOG_RAW
#define TM_LOG_RAW(a,b)
#else
#define TM_LOG_DTRACE TM_LOG_TRACE
#endif


static tm_mempool* mempool = NULL;
static tm_allocator allocator;
static size_t block_size;
static int thread_safe = 1;

int tm_block_init()
{
	int result = 1;

	block_size = sizeof(tm_block) + configuration.block_size;
	thread_safe = !(configuration.use_libev && configuration.return_pointers_through_pipes);

	if (configuration.use_mempool && !mempool) {
		mempool = tm_mempool_new(block_size, 1000, thread_safe);
		if (!mempool)
			result = 0;
	}

	if (result) {
		allocator.f_alloc = (tm_alloc_function) malloc;
		allocator.f_free = (tm_free_function) free;

		if (configuration.use_jemalloc) {
			allocator.f_alloc = (tm_alloc_function) je_malloc;
			allocator.f_free = (tm_free_function) je_free;
		}
	}

	return result;
}
void tm_block_fin()
{
	if (mempool) {
		tm_mempool_delete(mempool);
	}
}

inline void tm_block_destroy(tm_block *block)
{
	if(!block)
		return;
	
	tm_refcount_destroy((tm_refcount*)block);

	if (configuration.use_mempool && mempool)
		tm_mempool_return(mempool, (void *) block);
	else
		tm_free_custom(block, &allocator);

	TM_LOG_DTRACE("Block %p destroyed", block);
}

void tm_block_destructor(void *obj)
{
	tm_block_destroy((tm_block *)obj);
}

tm_block *tm_block_create()
{
	tm_block *block = NULL;

	if (configuration.use_mempool && mempool) {
		block = (tm_block*) tm_mempool_get(mempool);
	}
	else {
		block = (tm_block*) tm_calloc_custom(block_size, &allocator);
	}
	tm_refcount_init((tm_refcount*)block, tm_block_destructor);
	TM_LOG_DTRACE("Block %p created", block);
	return block;
}

tm_block *tm_block_copy(tm_block *block)
{
	tm_block *copy = NULL;

	if (!block)
		return NULL;

	copy = tm_block_create();
	if (!copy)
		return NULL;

	memcpy(copy->data, block->data, (size_t)configuration.block_size);

	TM_LOG_DTRACE("Block %p copyed to block %p", block, copy);

	return copy;
}

inline tm_block *tm_block_transfer_block(tm_block *block)
{
	if (!block)
		return NULL;

	if (configuration.copy_block_on_transfer)
		return tm_block_copy(block);
	else
		return (tm_block*)tm_refcount_retain((void*)block, thread_safe);
}

inline void tm_block_dispose_block(tm_block *block)
{
	if (!block)
		return;

	if (configuration.copy_block_on_transfer)
		tm_block_destroy(block);
	else
		tm_refcount_release((void*)block, thread_safe);

	TM_LOG_DTRACE("Block %p disposed", block);
}
