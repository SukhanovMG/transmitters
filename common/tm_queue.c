#include "tm_queue.h"
#include <stdlib.h>
#include "tm_alloc.h"
#include "tm_configuration.h"
#include <uthash/utlist.h>
#include <jemalloc/jemalloc.h>

static client_block_t tm_queue_pop_front_inner(tm_queue_ctx *q);
static int tm_queue_push_back_inner(tm_queue_ctx *q, client_block_t *block_queue_elem);

void tm_queue_destroy(tm_queue_ctx *q)
{
	if (q) {
		while (q->head) {
			tm_queue_pop_front_inner(q);
		}
		tm_free(q);
	}
}

tm_queue_ctx *tm_queue_create()
{
	tm_queue_ctx *q = tm_calloc(sizeof(tm_queue_ctx));
	if (!q){
		return q;
	}

	q->queue_elem_allocator.f_alloc = (tm_alloc_function) malloc;
	q->queue_elem_allocator.f_free = (tm_free_function) free;
	if (configuration.use_jemalloc) {
		q->queue_elem_allocator.f_alloc = (tm_alloc_function) je_malloc;
		q->queue_elem_allocator.f_free = (tm_free_function) je_free;
	}

	return q;
}

static int tm_queue_push_back_inner(tm_queue_ctx *q, client_block_t *client_block)
{
	tm_queue_elem_ctx *elem = NULL;

	if (!q || !client_block)
		return 0;

	elem = tm_calloc_custom(sizeof(tm_queue_elem_ctx), &q->queue_elem_allocator);
	if (!elem)
		return 0;

	elem->client_block = *client_block;

	DL_APPEND(q->head, elem);

	return 1;
}

static client_block_t tm_queue_pop_front_inner(tm_queue_ctx *q)
{
	tm_queue_elem_ctx *elem = NULL;
	client_block_t client_block = { NULL, 0 };
	if (q && q->head) {
		elem = q->head;
		DL_DELETE(q->head, elem);
		client_block = elem->client_block;
		tm_free_custom(elem, &q->queue_elem_allocator);
	}

	return client_block;
}

int tm_queue_push_back(tm_queue_ctx *q, client_block_t *client_block_array, int count)
{
	int result = 0;

	if (!q || !client_block_array || count <= 0)
		return result;

	for (int i = 0; i < count; i++) {
		result = tm_queue_push_back_inner(q, &client_block_array[i]);
		if (result != 1)
			break;
	}
	return result;
}

int tm_queue_pop_front(tm_queue_ctx *q, client_block_t *client_block_array, int count)
{
	int result = 0;

	if (!q || !client_block_array || count <= 0)
		return result;

	for (int i = 0; i < count; i++) {
		client_block_array[i] = tm_queue_pop_front_inner(q);
		if (client_block_array[i].block == NULL)
			break;
	}
	result = client_block_array[0].block != NULL;
	return result;
}

int tm_queue_is_empty(tm_queue_ctx *q)
{
	return q && !q->head;
}

