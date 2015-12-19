#include "tm_refcount.h"
#include "syslog.h"
#include "tm_logging.h"

inline void tm_refcount_init(tm_refcount *refcount_ctx, tm_refcount_destructor destructor)
{
	refcount_ctx->counter = 1;
	refcount_ctx->destructor = destructor;
}


static void *inner_tm_refcount_retain(void *obj, int thread_safe)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx) {
		if (thread_safe)
			__sync_add_and_fetch(&refcount_ctx->counter, 1);
		else
			refcount_ctx->counter++;
	}
	return refcount_ctx;
}

static void inner_tm_refcount_release(void *obj, int thread_safe)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx)
	{
		if (thread_safe) {
			if (__sync_sub_and_fetch(&refcount_ctx->counter, 1) == 0) {
				//if (refcount_ctx->destructor)
					refcount_ctx->destructor(obj);
			}
		} else {
			if (--refcount_ctx->counter == 0) {
				//if (refcount_ctx->destructor)
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
