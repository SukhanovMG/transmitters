#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

typedef union {
	uint64_t ui[2];
	struct {
		void *ptr;
		size_t count;
	} __attribute__ (( __aligned__( 16 ) ));
} counter_ptr;

typedef struct {
	void *value;
	counter_ptr next;
} lqueue_elem;

typedef struct {
	counter_ptr head, tail;
} lqueue;

lqueue *lqueue_new();
int lqueue_push_back(lqueue *q, void *val);
int lqueue_pop_front(lqueue *q, void **val);
void lqueue_delete(lqueue *q);