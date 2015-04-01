/**
 * @file common/transcoder_alloc.c
 * @date 2014
 * @copyright (c) ЗАО "Голлард"
 * @author Роман В. Косинский [armowl]
 * @brief Функции работы с памятью
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 700

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <src/multi-thread/copy-buffer/compat.h>
#include "transcoder_logging.h"

#include <src/multi-thread/copy-buffer/alloc.h>

#define TRANSCODER_ALLOC_MEMALIGN_USE 0

/**
 * Структура с контекстом буфера
 */
#if TRANSCODER_ALLOC_MEMALIGN_USE
typedef struct _transcoder_alloc_it {
	size_t length; /*!< Длина выделенного буфера */
}transcoder_alloc_it;

#define MEM_GUARD 64

#endif

/**
 * Выделение памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref transcoder_free())
 */
static transcoder_alloc_t transcoder_alloc_inner(size_t size)
{
#if !TRANSCODER_ALLOC_MEMALIGN_USE
	return malloc(size);
#else
	void *rc = NULL;
	posix_memalign(&rc, MEM_GUARD, (size) + sizeof(transcoder_alloc_it) + MEM_GUARD);
	if(rc) {
		((transcoder_alloc_it*)rc)->length = size;
		return (transcoder_alloc_t)(rc + sizeof(transcoder_alloc_it));
	}
	return NULL;
#endif
}

/**
 * Выделение памяти с очисткой выделенной памяти
 * @param size Запрашиваемый размер памяти
 * @return Указатель на выделенную память, NULL - при ошибке (освобождать при помощи @ref transcoder_free())
 */
static transcoder_alloc_t transcoder_calloc_inner(size_t size)
{
	void *rc;
	if ((rc = (void*)transcoder_alloc_inner(size))) {
		memset(rc, 0, size);
	}
	return (transcoder_alloc_t)rc;
}

/**
 * Освобождение выделенной памяти
 * @param ptr Указатель на выделенную память
 */
static void transcoder_free_inner(transcoder_alloc_t ptr)
{
	if (!ptr)
		return;
#if TRANSCODER_ALLOC_MEMALIGN_USE
	free(ptr - sizeof(transcoder_alloc_t));
#else
	free(ptr);
#endif
}

/**
 * Изменение размера буфера
 * @param memptr Указатель на существующий буфер
 * @param new_size Новый размер буфера
 * @return Указатель на выделенный буфер, NULL - при ошибке (освобождать при помощи @ref transcoder_free())
 */
static transcoder_alloc_t transcoder_realloc_inner(transcoder_alloc_t memptr, size_t new_size)
{
#if !TRANSCODER_ALLOC_MEMALIGN_USE
	return realloc(memptr, new_size);
#else
	void *rc = NULL;
	transcoder_alloc_it *bf;

	if(!memptr)
	return NULL;

	bf = (transcoder_alloc_it*)(memptr - sizeof(transcoder_alloc_it));

	if(bf->length >= new_size)
	return memptr;

	rc = (void*)transcoder_alloc_inner(new_size);
	if (rc) {
		bf = rc - sizeof(transcoder_alloc_it);
		bf->length = new_size;
		memcpy(bf + sizeof(transcoder_alloc_it), memptr, bf->length);
	}
	free(memptr - sizeof(transcoder_alloc_it));
	return (transcoder_alloc_t)rc;
#endif
}

/**
 * Функция делает копию строки в памяти
 * @param string Исходная строка
 * @param length Длина исходной строки, -1 - автоматическое определение длины
 * @return Указатель на копию (удаляеть при помощи @ref transcoder_free()), при ошибке - NULL
 */
static char *transcoder_strdup_inner(const char *string, int length)
{
	char *str = NULL;
	int len = length < 0 ? transcoder_strlen(string) : length;

	if (len && string) {
		str = (char*)transcoder_calloc_inner(len + 1);
		if (str)
			memcpy(str, string, len);
	}

	return str;
}


void _transcoder_free_d(transcoder_alloc_t ptr, int ln, char *file, const char *func, char *prm)
{
	syslog(LOG_DEBUG, "   FREE %s:[%d]:%s -> [%p] '%s'", file, ln, func, ptr, prm);
	transcoder_free_inner(ptr);
}
transcoder_alloc_t _transcoder_alloc_d(size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = transcoder_alloc_inner(size);
	syslog(LOG_DEBUG, "  ALLOC %s:[%d]:%s -> [%p](%"PRIuPTR") '%s'", file, ln, func, rc, size, prm);
	return (transcoder_alloc_t)rc;
}
transcoder_alloc_t _transcoder_calloc_d(size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = transcoder_calloc_inner(size);
	syslog(LOG_DEBUG, " CALLOC %s:[%d]:%s -> [%p](%"PRIuPTR") '%s'", file, ln, func, rc, size, prm);
	return (transcoder_alloc_t)rc;
}
transcoder_alloc_t _transcoder_realloc_d(transcoder_alloc_t ptr, size_t size, int ln, char *file, const char *func, char *prm)
{
	void *rc;
	rc = transcoder_realloc_inner(ptr, size);
	syslog(LOG_DEBUG, "REALLOC %s:[%d]:%s -> [%p]=>[%p](%"PRIuPTR") '%s'", file, ln, func, ptr, rc, size, prm);
	return (transcoder_alloc_t)rc;
}
char *_transcoder_strdup_d(const char *string, int length, int ln, char *file, const char *func, char *prm)
{
	char *res;
	res = transcoder_strdup_inner(string, length);
	syslog(LOG_DEBUG, " STRDUP %s:[%d]:%s -> [%p] '%s'", file, ln, func, res, prm);
	return res;
}


transcoder_alloc_t _transcoder_alloc(size_t size)
{
	return transcoder_alloc_inner(size);
}

transcoder_alloc_t _transcoder_calloc(size_t size)
{
	return transcoder_calloc_inner(size);
}

transcoder_alloc_t _transcoder_realloc(transcoder_alloc_t memptr, size_t new_size)
{
	return transcoder_realloc_inner(memptr, new_size);
}

void _transcoder_free(transcoder_alloc_t ptr)
{
	transcoder_free_inner(ptr);
}

char *_transcoder_strdup(const char *string, int length)
{
	return transcoder_strdup_inner(string, length);
}

