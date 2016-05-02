#include "tm_queue_rbuf.h"

#include "tm_configuration.h"

#define RBUF_LENGTH 4000000

typedef struct _queue_rbuf {
	client_block_t *buffer;
	size_t length;
	size_t count;
	size_t write;
	size_t read;
} queue_rbuf;

void tm_queue_destroy_rbuf(void *_q)
{
	queue_rbuf *q = (queue_rbuf *)_q;
	if (q) {
		tm_free(q->buffer);
		tm_free(q);
	}
}

void *tm_queue_create_rbuf()
{
	queue_rbuf *q = tm_calloc(sizeof(queue_rbuf));
	if (!q){
		return NULL;
	}
	q->length = RBUF_LENGTH;
	q->buffer = tm_calloc(sizeof(client_block_t) * q->length);
	if (!q->buffer) {
		tm_free(q);
		return NULL;
	}
	return (void *) q;
}

int tm_queue_push_back_rbuf(void *_q, client_block_t *client_block)
{
	queue_rbuf *q = (queue_rbuf *)_q;

	if (!q || !client_block)
		return 0;

	if (q->count == q->length)
		return 0;

	q->buffer[q->write] = *client_block;
	q->count++;
	if (q->write == (q->length - 1)) {
		q->write = 0;
	} else {
		q->write++;
	}

	return 1;
}

client_block_t tm_queue_pop_front_rbuf(void *_q)
{
	queue_rbuf *q = (queue_rbuf *)_q;
	client_block_t client_block = { NULL, 0 };
	if (q->count > 0) {
		client_block = q->buffer[q->read];
		q->count--;
		if (q->read == (q->length - 1)) {
			q->read = 0;
		} else {
			q->read++;
		}
	}

	return client_block;
}

int tm_queue_is_empty_rbuf(void *_q)
{
	queue_rbuf *q = (queue_rbuf *)_q;
	return q && q->count == 0;
}
