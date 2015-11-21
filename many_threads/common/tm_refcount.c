#include "tm_refcount.h"
#include "syslog.h"

inline void tm_refcount_init(tm_refcount *refcount_ctx, tm_refcount_destructor destructor)
{
	refcount_ctx->counter = 1;
	refcount_ctx->destructor = destructor;
}


static void *inner_tm_refcount_retain(void *obj)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx)
		__sync_add_and_fetch(&refcount_ctx->counter, 1);
	return refcount_ctx;
}

static void inner_tm_refcount_release(void *obj)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	if (refcount_ctx)
	{
		if (__sync_sub_and_fetch(&refcount_ctx->counter, 1) == 0)
			if (refcount_ctx->destructor)
				refcount_ctx->destructor(obj);
	}
}

inline void *_tm_refcount_retain_d(void *obj, int ln, char *file, const char *func)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	syslog(LOG_DEBUG, " RET %s:[%d]:%s -> [%p] (%d->%d)", file, ln, func, obj, refcount_ctx->counter, refcount_ctx->counter + 1);
	return inner_tm_refcount_retain(obj);
}

inline void _tm_refcount_release_d(void *obj, int ln, char *file, const char *func)
{
	tm_refcount *refcount_ctx = (tm_refcount*)obj;
	syslog(LOG_DEBUG, " REL %s:[%d]:%s -> [%p] (%d->%d)", file, ln, func, obj, refcount_ctx->counter, refcount_ctx->counter - 1);
	inner_tm_refcount_release(obj);
}

inline void *_tm_refcount_retain(void *obj)
{
	return inner_tm_refcount_retain(obj);
}

inline void _tm_refcount_release(void *obj)
{
	inner_tm_refcount_release(obj);
}
