#ifndef TM_ALLOC_H
#define TM_ALLOC_H

#include <unistd.h>

typedef void * tm_alloc_t;

#define tm_alloc(size) _tm_alloc(size)
#define tm_calloc(size) _tm_calloc(size)
#define tm_realloc(ptr, new_size) _tm_realloc(ptr, new_size)
#define tm_free(tm_alloc) _tm_free(tm_alloc)
#define tm_strdup(ptr, length) _tm_strdup(ptr, length)


tm_alloc_t _tm_alloc(size_t);
tm_alloc_t _tm_calloc(size_t);
tm_alloc_t _tm_realloc(void *memptr, size_t new_size);
void _tm_free(tm_alloc_t);
char *_tm_strdup(const char *string, int length);


#endif
