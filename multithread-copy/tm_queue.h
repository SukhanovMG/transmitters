
#ifndef TM_QUEUE_H
#define TM_QUEUE_H

#include <pthread.h>
#include "tm_alloc.h"

// Вынести в конфиг
#define QUEUE_SIZE 10

typedef struct _tm_queue_t {

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	void **buffers;
	int size;
	int in;
	int out;

} tm_queue_t;

tm_queue_t* tm_queue_create();
void tm_queue_destroy(tm_queue_t *q);
int tm_queue_push_back(tm_queue_t *q);
void* tm_queue_pop_front(tm_queue_t *q);

#endif /* TM_QUEUE_H */
