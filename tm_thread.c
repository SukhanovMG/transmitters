#include "tm_thread.h"

#include "tm_thread_simple.h"
#include "tm_thread_events.h"
#include "tm_thread_events_queue.h"
#include "tm_configuration.h"
#include "tm_logging.h"

typedef TMThreadStatus(*threads_init_function)(int);
typedef TMThreadStatus(*threads_work_function)();
typedef TMThreadStatus(*threads_shutdown_function)();


typedef struct {
	threads_init_function init;
	threads_work_function work;
	threads_shutdown_function shutdown;
} thread_module_ctx_t;

thread_module_ctx_t thread_module_ctx;

const char * tm_threads_type_to_str(TMThreadType type)
{
	switch (type) {
		case TMThreadType_Simple:
			return "simple";
		case TMThreadType_Libev_queue:
			return "libev_queue";
		case TMThreadType_Libev_pipe:
		default:
			return "libev_pipe";
	}
}

TMThreadStatus tm_threads_init(int count)
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	switch (configuration.thread_module_type) {
		case TMThreadType_Simple:
			thread_module_ctx.init = tm_threads_init_simple;
			thread_module_ctx.work = tm_threads_work_simple;
			thread_module_ctx.shutdown = tm_threads_shutdown_simple;
			break;
		case TMThreadType_Libev_queue:
			thread_module_ctx.init = tm_threads_init_events_queue;
			thread_module_ctx.work = tm_threads_work_events_queue;
			thread_module_ctx.shutdown = tm_threads_shutdown_events_queue;
			break;
		case TMThreadType_Libev_pipe:
			thread_module_ctx.init = tm_threads_init_events;
			thread_module_ctx.work = tm_threads_work_events;
			thread_module_ctx.shutdown = tm_threads_shutdown_events;
		default:
			break;
	}


	if (thread_module_ctx.init)
		status = thread_module_ctx.init(count);
	else
		TM_LOG_ERROR("Thread module initialization fail.");

	return status;
}

TMThreadStatus tm_threads_work()
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if (thread_module_ctx.work)
		status = thread_module_ctx.work();
	else
		TM_LOG_ERROR("Thread module work fail.");

	return status;
}

TMThreadStatus tm_threads_shutdown()
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if (thread_module_ctx.shutdown)
		status = thread_module_ctx.shutdown();
	else
		TM_LOG_ERROR("Thread module shutdown fail.");

	return status;
}
