#ifndef TM_REFCOUNT_H
#define TM_REFCOUNT_H

#define TM_REFCOUNT_DEBUG 1

typedef void (*tm_refcount_destructor)(void *);

typedef struct _tm_refcount {
	volatile unsigned int counter;
	tm_refcount_destructor destructor;
} tm_refcount;

void tm_refcount_init(tm_refcount *, tm_refcount_destructor);

#if TM_REFCOUNT_DEBUG
#define tm_refcount_retain(obj) _tm_refcount_retain_d(obj, __LINE__, __FILE__, __FUNCTION__)
#define tm_refcount_release(obj) _tm_refcount_release_d(obj, __LINE__, __FILE__, __FUNCTION__)
#else
#define tm_refcount_retain(obj) _tm_refcount_retain(obj)
#define tm_refcount_release(obj) _tm_refcount_release(obj)
#endif

void *_tm_refcount_retain_d(void *, int, char*, const char*);
void _tm_refcount_release_d(void *, int, char*, const char*);
void *_tm_refcount_retain(void *);
void _tm_refcount_release(void *);

#endif /* TM_REFCOUNT_H */
