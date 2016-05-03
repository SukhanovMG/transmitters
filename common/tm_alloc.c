#include "tm_alloc.h"

#include <stdlib.h>
#include <string.h>


/**
 * Выделение памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static void* tm_alloc_inner(size_t size)
{
	void *ptr = NULL;
	ptr = malloc(size);
	return ptr;
}

/**
 * Выделение памяти с очисткой выделенной памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static void* tm_calloc_inner(size_t size)
{
	void *ptr = NULL;
	ptr = tm_alloc_inner(size);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}

/**
 * Освобождение выделенной памяти
 * @param ptr Указатель на выделенную память
 */
static void tm_free_inner(void *ptr)
{
	if (ptr) {
		free(ptr);
	}
}

/**
 * Изменение размера буфера
 * @param memptr Указатель на существующий буфер
 * @param new_size Новый размер буфера
 * @return Указатель на выделенный буфер, NULL - при ошибке (освобождать при помощи @ref tm_free())
 */
static void* tm_realloc_inner(void *memptr, size_t new_size)
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

	if (string) {
		size_t len = length < 0 ? strlen(string) : (size_t)length;
		if (len) {
			str = (char *) tm_calloc_inner(len + 1);
			if (str)
				memcpy(str, string, len);
		}
	}

	return str;
}

void* _tm_alloc(size_t size)
{
	return tm_alloc_inner(size);
}

void* _tm_calloc(size_t size)
{
	return tm_calloc_inner(size);
}

void* _tm_realloc(void *memptr, size_t new_size)
{
	return tm_realloc_inner(memptr, new_size);
}

void _tm_free(void *ptr)
{
	tm_free_inner(ptr);
}

char *_tm_strdup(const char *string, int length)
{
	return tm_strdup_inner(string, length);
}

