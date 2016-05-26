#include "tm_refcount.h"
#include "syslog.h"
#include "tm_configuration.h"

// Инициализация счётчика ссылок
inline void tm_refcount_init(tm_refcount *refcount_ctx, tm_refcount_destructor destructor)
{
	refcount_ctx->counter = 1;
	refcount_ctx->destructor = destructor;
	refcount_ctx->w_mutex = 0;
	if (configuration.refcount_with_mutex) {
		pthread_mutex_init(&refcount_ctx->mutex, NULL);
		refcount_ctx->w_mutex = 1;
	}
}

// Удаление счётчика ссылок
inline void tm_refcount_destroy(tm_refcount *refcount_ctx)
{
	if (configuration.refcount_with_mutex)
		pthread_mutex_destroy(&refcount_ctx->mutex);
}

// Инкрементация счётчика с помощью атомарной операции
static void retain_w_atomic(tm_refcount *refcount_ctx)
{
	__sync_fetch_and_add(&refcount_ctx->counter, 1);
}

// Инекрементация счётчика с помощью мьютекса и обычного оператора
static void retain_w_mutex(tm_refcount *refcount_ctx)
{
	pthread_mutex_lock(&refcount_ctx->mutex);
	refcount_ctx->counter++;
	pthread_mutex_unlock(&refcount_ctx->mutex);
}

// Инкрементация счётчика ссылок
static void *inner_tm_refcount_retain(void *obj, int thread_safe)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx) {
		if (thread_safe) {
			if (refcount_ctx->w_mutex)
				retain_w_mutex(refcount_ctx);
			else
				retain_w_atomic(refcount_ctx);
		}
		else
			refcount_ctx->counter++;
	}
	return refcount_ctx;
}

// Декрементация счётчика ссылок с помощью атомарной операции
// и удаление данных если нужно
static void release_w_atomic(tm_refcount *refcount_ctx)
{
	if (__sync_sub_and_fetch(&refcount_ctx->counter, 1) == 0) {
		if (refcount_ctx->destructor)
			refcount_ctx->destructor((void*)refcount_ctx);
	}
}

// Декрементация счётчика ссылок с помощью мьютекса и обычного оператора
// и удаление данных если нужно
static void release_w_mutex(tm_refcount* refcount_ctx)
{
	pthread_mutex_lock(&refcount_ctx->mutex);
	unsigned int cnt = --refcount_ctx->counter;
	pthread_mutex_unlock(&refcount_ctx->mutex);
	if (cnt == 0) {
		if (refcount_ctx->destructor)
			refcount_ctx->destructor((void*)refcount_ctx);
	}
}

// Декрементация счётчика ссылок
// и удаление данных если нужно
static void inner_tm_refcount_release(void *obj, int thread_safe)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx)
	{
		if (thread_safe) {
			if (refcount_ctx->w_mutex)
				release_w_mutex(refcount_ctx);
			else
				release_w_atomic(refcount_ctx);
		} else {
			if (--refcount_ctx->counter == 0) {
				if (refcount_ctx->destructor)
					refcount_ctx->destructor(obj);
			}
		}
	}
}

inline void *_tm_refcount_retain_d(void *obj, int thread_safe, int ln, char *file, const char *func)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	syslog(LOG_DEBUG, " RET %s:[%d]:%s -> [%p] (%d->%d)", file, ln, func, obj, refcount_ctx->counter, refcount_ctx->counter + 1);
	return inner_tm_refcount_retain(obj, thread_safe);
}

inline void _tm_refcount_release_d(void *obj, int thread_safe, int ln, char *file, const char *func)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	syslog(LOG_DEBUG, " REL %s:[%d]:%s -> [%p] (%d->%d)", file, ln, func, obj, refcount_ctx->counter, refcount_ctx->counter - 1);
	inner_tm_refcount_release(obj, thread_safe);
}

inline void *_tm_refcount_retain(void *obj, int thread_safe)
{
	return inner_tm_refcount_retain(obj, thread_safe);
}

inline void _tm_refcount_release(void *obj, int thread_safe)
{
	inner_tm_refcount_release(obj, thread_safe);
}
