#ifndef TM_THREAD_H
#define TM_THREAD_H

typedef enum _TMThreadStatus {
	TMThreadStatus_SUCCESS = 0,
	TMThreadStatus_ERROR = 1
} TMThreadStatus;

typedef enum _TMThreadType {
	TMThreadType_Simple = 0,
	TMThreadType_Libev_queue,
	TMThreadType_Libev_pipe
} TMThreadType;

const char * tm_threads_type_to_str(TMThreadType type);

TMThreadStatus tm_threads_init(int count);
TMThreadStatus tm_threads_shutdown();
TMThreadStatus tm_threads_work();

#endif /* TM_THREAD_H */
