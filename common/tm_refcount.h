#ifndef TM_REFCOUNT_H
#define TM_REFCOUNT_H

#define TM_REFCOUNT_DEBUG 0

#include <pthread.h>

typedef void (*tm_refcount_destructor)(void *);

typedef struct _tm_refcount {
	volatile unsigned int counter;
	tm_refcount_destructor destructor;
	int w_mutex;
	pthread_mutex_t mutex;
} tm_refcount;

void tm_refcount_init(tm_refcount *, tm_refcount_destructor);
void tm_refcount_destroy(tm_refcount *);

#if TM_REFCOUNT_DEBUG
#define tm_refcount_retain(obj, thread_safe) _tm_refcount_retain_d(obj, thread_safe, __LINE__, __FILE__, __FUNCTION__)
#define tm_refcount_release(obj, thread_safe) _tm_refcount_release_d(obj, thread_safe, __LINE__, __FILE__, __FUNCTION__)
#else
#define tm_refcount_retain(obj, thread_safe) _tm_refcount_retain(obj, thread_safe)
#define tm_refcount_release(obj, thread_safe) _tm_refcount_release(obj, thread_safe)
#endif

void *_tm_refcount_retain_d(void *, int, int, char*, const char*);
void _tm_refcount_release_d(void *, int, int, char*, const char*);
void *_tm_refcount_retain(void *, int);
void _tm_refcount_release(void *, int);

#endif /* TM_REFCOUNT_H */
