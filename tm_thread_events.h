#ifndef TM_THREAD_EVENTS_H
#define TM_THREAD_EVENTS_H

#include "tm_thread.h"

TMThreadStatus tm_threads_init_events(int count);
TMThreadStatus tm_threads_shutdown_events();
TMThreadStatus tm_threads_work_events();

#endif /* TM_THREAD_EVENTS_H */
