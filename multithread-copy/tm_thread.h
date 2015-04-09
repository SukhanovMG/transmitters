#ifndef TM_THREAD_H
#define TM_THREAD_H

typedef enum _TMThreadStatus {
	TMThreadStatus_SUCCESS = 0,
	TMThreadStatus_ERROR = 1
} TMThreadStatus;

TMThreadStatus tm_threads_init(int count);
TMThreadStatus tm_threads_shutdown();

#endif /* TM_THREAD_H */
