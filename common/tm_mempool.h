/*
 * Работа с пулом
 */

#ifndef TM_MEMPOOL_H
#define TM_MEMPOOL_H

#include <unistd.h>

// Пул возвращается как void*
typedef void tm_mempool;

// Удалить пул
void tm_mempool_delete(tm_mempool *pool);
// Создать пул
tm_mempool *tm_mempool_new(size_t elem_size, size_t count, int thread_safe);
// Получить блок из пула
void *tm_mempool_get(tm_mempool *pool);
// Вернуть блок в пул
void tm_mempool_return(tm_mempool *pool, void *data);

#endif
