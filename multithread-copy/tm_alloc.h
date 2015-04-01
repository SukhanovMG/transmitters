#ifndef ALLOC_H
#define ALLOC_H

#include <unistd.h>

typedef void * alloc_t;

#define alloc(size) _alloc(size)
#define calloc(size) _calloc(size)
#define realloc(ptr, new_size) _realloc(ptr, new_size)
#define free(alloc) _free(alloc)
#define strdup(ptr, length) _strdup(ptr, length)

alloc_t _alloc(size_t);
alloc_t _calloc(size_t);
alloc_t _realloc(void *memptr, size_t new_size);
void _free(alloc_t);
char *_strdup(const char *string, int length);


#endif
