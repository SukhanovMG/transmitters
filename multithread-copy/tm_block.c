#include "tm_block.h"

#include "tm_alloc.h"
#include "tm_read_config.h"

void tm_block_destroy(tm_block *block)
{
	if(!block)
		return;
	tm_free(block->block);
	tm_free(block);
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
	return block;
}
