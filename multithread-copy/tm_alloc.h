#ifndef TM_ALLOC_H
#define TM_ALLOC_H

#include <unistd.h>

typedef void * tm_alloc_t;


#define TM_CHECK_ALLOC_DEBUG 1

#if TM_CHECK_ALLOC_DEBUG
#define tm_alloc(size) _tm_alloc_d(size, __LINE__, __FILE__, __FUNCTION__, TM_LOG_STR(size))
#define tm_calloc(size) _tm_calloc_d(size, __LINE__, __FILE__, __FUNCTION__, TM_LOG_STR(size))
#define tm_realloc(ptr, new_size) _tm_realloc_d(ptr, new_size, __LINE__, __FILE__, __FUNCTION__, TM_LOG_STR(new_size))
#define tm_free(tm_alloc) _tm_free_d(tm_alloc, __LINE__, __FILE__, __FUNCTION__, TM_LOG_STR(tm_alloc))
#define tm_strdup(ptr, length) _tm_strdup_d(ptr, length, __LINE__, __FILE__, __FUNCTION__, TM_LOG_STR(length))
#else
#define tm_alloc(size) _tm_alloc(size)
#define tm_calloc(size) _tm_calloc(size)
#define tm_realloc(ptr, new_size) _tm_realloc(ptr, new_size)
#define tm_free(tm_alloc) _tm_free(tm_alloc)
#define tm_strdup(ptr, length) _tm_strdup(ptr, length)
#endif

tm_alloc_t _tm_alloc_d(size_t, int, char*, const char*, char*);
tm_alloc_t _tm_calloc_d(size_t, int, char*, const char*, char*);
tm_alloc_t _tm_realloc_d(void *memptr, size_t new_size, int, char*, const char*, char*);
void _tm_free_d(tm_alloc_t, int, char*, const char*, char*);
char *_tm_strdup_d(const char *string, int length, int, char*, const char*, char*);
tm_alloc_t _tm_alloc(size_t);
tm_alloc_t _tm_calloc(size_t);
tm_alloc_t _tm_realloc(void *memptr, size_t new_size);
void _tm_free(tm_alloc_t);
char *_tm_strdup(const char *string, int length);


#endif
