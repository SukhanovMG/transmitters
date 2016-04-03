#ifndef TM_THREAD_EVENTS_LOCLESS_QUEUE_H
#define TM_THREAD_EVENTS_LOCLESS_QUEUE_H

#include "tm_thread.h"

TMThreadStatus tm_threads_init_events_lockless_queue(int count);
TMThreadStatus tm_threads_shutdown_events_lockless_queue();
TMThreadStatus tm_threads_work_events_lockless_queue();

#endif /* TM_THREAD_EVENTS_LOCLESS_QUEUE_H */
