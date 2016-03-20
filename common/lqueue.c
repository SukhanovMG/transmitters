#include "lqueue.h"

#include <stdlib.h>

int cas(counter_ptr *addr, counter_ptr *cmp, counter_ptr *nval)
{
	char result;
	__asm__ __volatile__ (
		"lock cmpxchg16b %1\n\t"
		"setz %0\n"
		: "=q" (result)
		, "+m" (addr->ui)
		: "a" (cmp->ptr), "d" (cmp->count)
		, "b" (nval->ptr), "c" (nval->count)
		: "cc"
	);
	return (int)result;
}

lqueue_elem *lqueue_elem_new(void *val)
{
	lqueue_elem *qe = calloc(1, sizeof(lqueue_elem));
	if(qe) {
		qe->value = val;
	}
	return qe;
}

lqueue *lqueue_new()
{
	lqueue *q = calloc(1, sizeof(lqueue));
	if (q) {
		lqueue_elem *qe = lqueue_elem_new(0);
		if (qe) {
			q->head.ptr = (void*) qe;
			q->tail.ptr = (void*) qe;
		} else {
			free(q);
			q = NULL;
		}
	}
	return q;
}

int lqueue_push_back(lqueue *q, void *val)
{
	if (q) {
		lqueue_elem *qe = lqueue_elem_new(val);
		if (!qe)
			return 0;
		counter_ptr tail, next;
		while(1) {
			tail = q->tail;
			next = ((lqueue_elem*)tail.ptr)->next;
			if (tail.ptr == q->tail.ptr && tail.count == q->tail.count) {
				if (next.ptr == NULL) {
					counter_ptr new;
					new.ptr = (void*) qe;
					new.count = next.count + 1;
					if (cas(&((lqueue_elem*)tail.ptr)->next, &next, &new))
						break;
				} else {
					counter_ptr new;
					new.ptr = next.ptr;
					new.count = tail.count + 1;
					cas(&q->tail, &tail, &new);
				}
			}
		}
		counter_ptr new;
		new.ptr = (void*) qe;
		new.count = tail.count + 1;
		cas(&q->tail, &tail, &new);
		return 1;
	}
	return 0;
}

int lqueue_pop_front(lqueue *q, void **val)
{
	if (q) {
		counter_ptr head, tail, next;
		while(1) {
			head = q->head;
			tail = q->tail;
			next = ((lqueue_elem*)head.ptr)->next;
			if (head.ptr == q->head.ptr && head.count == q->head.count) {
				if (head.ptr == tail.ptr) {
					counter_ptr new;
					new.ptr = next.ptr;
					new.count = tail.count + 1;
					if (next.ptr == NULL) return 0;
					cas(&q->tail, &tail, &new);
				} else {
					counter_ptr new;
					new.ptr = next.ptr;
					new.count = tail.count + 1;
					*val = ((lqueue_elem*)next.ptr)->value;
					if (cas(&q->head, &head, &new))
						break;
				}
			}
		}
		free(head.ptr);
		return 1;
	}
	return 0;
}

void lqueue_delete(lqueue *q)
{
	if (q) {
		int pop_res = 1;
		while (pop_res) {
			void *dummy;
			pop_res = lqueue_pop_front(q, &dummy);
			(void)dummy;
		}
		free(q->head.ptr);
		free(q);
	}
}