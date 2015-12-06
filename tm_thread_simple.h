#ifndef TM_THREAD_SIMPLE_H
#define TM_THREAD_SIMPLE_H

#include "tm_thread.h"

TMThreadStatus tm_threads_init_simple(int count);
TMThreadStatus tm_threads_shutdown_simple();
TMThreadStatus tm_threads_work_simple();

#endif /* TM_THREAD_SIMPLE_H */
