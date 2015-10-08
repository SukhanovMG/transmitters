#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tm_alloc.h"
#include "tm_compat.h"
#include "tm_logging.h"
#include "tm_read_config.h"


/**
 * Выделение памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static tm_alloc_t tm_alloc_inner(size_t size, tm_allocator *allocator)
{
	void *ptr = NULL;

	if (allocator) {
		ptr = allocator->f_alloc(size);
	} else {
		ptr = malloc(size);
	}

	return ptr;
}

/**
 * Выделение памяти с очисткой выделенной памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static tm_alloc_t tm_calloc_inner(size_t size, tm_allocator *allocator)
{
	void *ptr = NULL;
	ptr = tm_alloc_inner(size, allocator);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}

/**
 * Освобождение выделенной памяти
 * @param ptr Указатель на выделенную память
 */
static void tm_free_inner(tm_alloc_t ptr, tm_allocator *allocator)
{
	if (ptr) {
		if (allocator) {
			allocator->f_free(ptr);
		} else {
			free(ptr);
		}
	}
}

/**
 * Изменение размера буфера
 * @param memptr Указатель на существующий буфер
 * @param new_size Новый размер буфера
 * @return Указатель на выделенный буфер, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static tm_alloc_t tm_realloc_inner(tm_alloc_t memptr, size_t new_size)
{
	return realloc(memptr, new_size);
}

/**
 * Функция делает копию строки в памяти
 * @param string Исходная строка
 * @param length Длина исходной строки, -1 - автоматическое определение длины
 * @return Указатель на копию (удаляеть при помощи @ref tm_free()), при ошибке - NULL
 */
static char *tm_strdup_inner(const char *string, int length)
{
	char *str = NULL;
	int len = length < 0 ? tm_strlen(string) : length;

	if (len && string) {
		str = (char*)tm_calloc_inner(len + 1, NULL);
		if (str)
			memcpy(str, string, len);
	}

	return str;
}


void _tm_free_d(tm_alloc_t ptr, int ln, char *file, const char *func, char *prm)
{
	syslog(LOG_DEBUG, "   FREE %s:[%d]:%s -> [%p] '%s'", file, ln, func, ptr, prm);
	tm_free_inner(ptr, NULL);
}
tm_alloc_t _tm_alloc_d(size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = tm_alloc_inner(size, NULL);
	syslog(LOG_DEBUG, "  ALLOC %s:[%d]:%s -> [%p](%"PRIuPTR") '%s'", file, ln, func, rc, size, prm);
	return (tm_alloc_t)rc;
}
tm_alloc_t _tm_calloc_d(size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = tm_calloc_inner(size, NULL);
	syslog(LOG_DEBUG, " CALLOC %s:[%d]:%s -> [%p](%"PRIuPTR") '%s'", file, ln, func, rc, size, prm);
	return (tm_alloc_t)rc;
}
tm_alloc_t _tm_realloc_d(tm_alloc_t ptr, size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = tm_realloc_inner(ptr, size);
	syslog(LOG_DEBUG, "REALLOC %s:[%d]:%s -> [%p]=>[%p](%"PRIuPTR") '%s'", file, ln, func, ptr, rc, size, prm);
	return (tm_alloc_t)rc;
}
char *_tm_strdup_d(const char *string, int length, int ln, char *file, const char *func, char *prm)
{
	char *res;
	res = tm_strdup_inner(string, length);
	syslog(LOG_DEBUG, " STRDUP %s:[%d]:%s -> [%p] '%s'", file, ln, func, res, prm);
	return res;
}


tm_alloc_t _tm_alloc(size_t size)
{
	return tm_alloc_inner(size, NULL);
}

tm_alloc_t _tm_alloc_custom(size_t size, tm_allocator *allocator)
{
	return tm_alloc_inner(size, allocator);
}

tm_alloc_t _tm_calloc(size_t size)
{
	return tm_calloc_inner(size, NULL);
}

tm_alloc_t _tm_calloc_custom(size_t size, tm_allocator *allocator)
{
	return tm_calloc_inner(size, allocator);
}

tm_alloc_t _tm_realloc(tm_alloc_t memptr, size_t new_size)
{
	return tm_realloc_inner(memptr, new_size);
}

void _tm_free(tm_alloc_t ptr)
{
	tm_free_inner(ptr, NULL);
}

void _tm_free_custom(tm_alloc_t ptr, tm_allocator *allocator)
{
	tm_free_inner(ptr, allocator);
}

char *_tm_strdup(const char *string, int length)
{
	return tm_strdup_inner(string, length);
}

