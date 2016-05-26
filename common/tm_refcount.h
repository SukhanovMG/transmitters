/*
 * Модуль для работы со счётчиком ссылок
 * */

#ifndef TM_REFCOUNT_H
#define TM_REFCOUNT_H

#define TM_REFCOUNT_DEBUG 0

#include <pthread.h>

// Определение типа указателя на функцию для деструктора
typedef void (*tm_refcount_destructor)(void *);

// Структура счётчика ссылок
typedef struct _tm_refcount {
	volatile unsigned int counter; // Сам счётчик
	tm_refcount_destructor destructor; // Указатель на функцию для удаления данных
	int w_mutex; // Флаг использования мьютекса
	pthread_mutex_t mutex; // мьютекс (если используется)
} tm_refcount;

// Инициализация счётчика ссылок
void tm_refcount_init(tm_refcount *, tm_refcount_destructor);
// Удаление счётчика ссылок
void tm_refcount_destroy(tm_refcount *);

#if TM_REFCOUNT_DEBUG
#define tm_refcount_retain(obj, thread_safe) _tm_refcount_retain_d(obj, thread_safe, __LINE__, __FILE__, __FUNCTION__)
#define tm_refcount_release(obj, thread_safe) _tm_refcount_release_d(obj, thread_safe, __LINE__, __FILE__, __FUNCTION__)
#else
#define tm_refcount_retain(obj, thread_safe) _tm_refcount_retain(obj, thread_safe)
#define tm_refcount_release(obj, thread_safe) _tm_refcount_release(obj, thread_safe)
#endif

void *_tm_refcount_retain_d(void *, int, int, char*, const char*);
void _tm_refcount_release_d(void *, int, int, char*, const char*);
void *_tm_refcount_retain(void *, int);
void _tm_refcount_release(void *, int);

#endif /* TM_REFCOUNT_H */
