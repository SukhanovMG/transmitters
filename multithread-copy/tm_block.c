#include "tm_block.h"

#include "tm_alloc.h"
#include "tm_read_config.h"
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

int tm_block_init()
{
	int result = 0;

	if (!mempool) {
		mempool = tm_mempool_new(configuration.block_size, 4096);
		if (mempool)
			result = 1;
	}
	return result;
}
void tm_block_fin()
{
	if (mempool) {
		tm_mempool_delete(mempool);
	}
}

void tm_block_destroy(tm_block *block)
{
	if(!block)
		return;
	if (block->block) {
		if (configuration.use_mempool && mempool)
			tm_mempool_return(mempool, block->block);
		else if (configuration.use_jemalloc)
			je_free(block->block);
		else
			tm_free(block->block);
	}
	if (configuration.use_jemalloc)
		je_free(block);
	else
		tm_free(block);

	TM_LOG_DTRACE("Block %p destroyed", block);
}

void tm_block_destructor(void *obj)
{
	tm_block_destroy((tm_block *)obj);
}

tm_block *tm_block_create()
{
	tm_block *block = NULL;
	if (configuration.use_jemalloc)
		block = je_malloc(sizeof(tm_block));
	else
		block = tm_alloc(sizeof(tm_block));

	if (configuration.use_mempool && mempool) {
		block->block = tm_mempool_get(mempool);
	}
	else if (configuration.use_jemalloc) {
		block->block = je_calloc(1, configuration.block_size);
	}
	else {
		block->block = tm_calloc(configuration.block_size);
	}

	if (!block->block) {
		TM_LOG_DTRACE("Failed to create block");
		return NULL;
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

	memcpy(copy->block, block->block, configuration.block_size);

	TM_LOG_DTRACE("Block %p copyed to block %p", block, copy);

	return copy;
}

tm_block *tm_block_transfer_block(tm_block *block)
{
	if (!block)
		return NULL;

	if (configuration.copy_block_on_transfer)
		return tm_block_copy(block);
	else
		return (tm_block*)tm_refcount_retain((void*)block);
}

void tm_block_dispose_block(tm_block *block)
{
	if (!block)
		return;

	if (configuration.copy_block_on_transfer)
		tm_block_destroy(block);
	else
		tm_refcount_release((void*)block);
}
