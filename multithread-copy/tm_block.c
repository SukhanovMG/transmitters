#include "tm_block.h"

#include "tm_alloc.h"
#include "tm_read_config.h"
#include "tm_logging.h"

void tm_block_destroy(tm_block *block)
{
	if(!block)
		return;
	tm_free(block->block);
	tm_free(block);
	//TM_LOG_TRACE("block %p destroyed", block);
}

void tm_block_destructor(void *obj)
{
	tm_block_destroy((tm_block *)obj);
}

tm_block *tm_block_create()
{
	tm_block *block = NULL;
	block = tm_alloc(sizeof(tm_block));
	block->block = tm_calloc(configuration.block_size);
	if (!block->block)
		return NULL;

	tm_refcount_init((tm_refcount*)block, tm_block_destructor);
	//TM_LOG_TRACE("block %p created", block);
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

	//TM_LOG_TRACE("block %p copyed to %p", block, copy);

	return copy;
}

tm_block *tm_block_transfer_block(tm_block *block)
{
	if (!block)
		return NULL;

#ifdef TM_BLOCK_COPY_ON_TRANSFER
	return tm_block_copy(block);
#else
	return (tm_block*)tm_refcount_retain((void*)block);
#endif
}

void tm_block_dispose_block(tm_block *block)
{
	if (!block)
		return;

#ifdef TM_BLOCK_COPY_ON_TRANSFER
	tm_block_destroy(block);
#else
	tm_refcount_release((void*)block);
#endif
}
