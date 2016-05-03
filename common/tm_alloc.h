#ifndef TM_ALLOC_H
#define TM_ALLOC_H

#include <stddef.h>

#define tm_alloc(size) _tm_alloc(size)
#define tm_calloc(size) _tm_calloc(size)
#define tm_realloc(ptr, new_size) _tm_realloc(ptr, new_size)
#define tm_free(tm_alloc) _tm_free(tm_alloc)
#define tm_strdup(ptr, length) _tm_strdup(ptr, length)

void* _tm_alloc(size_t);
void* _tm_calloc(size_t);
void* _tm_realloc(void *memptr, size_t new_size);
void _tm_free(void *);
char *_tm_strdup(const char *string, int length);


#endif
