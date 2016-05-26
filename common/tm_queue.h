/*
 * Модуль для работы с очередью
 */

#ifndef TM_QUEUE_H
#define TM_QUEUE_H

#include "tm_block.h"
#include "tm_alloc.h"

// Типы очредей
typedef enum {
	kTmQueueSimple = 0, // Очередь на списке
	kTmQueueSimpleMempool, // Очередь на списке с использованием пула
	kTmQueueRbuf // Очередь на циклическом буфере
} tm_queue_type;

// Очередь отдаётся в виде указателя void*
typedef void tm_queue_ctx;

// Создать очередь заданного типа
tm_queue_ctx *tm_queue_create(tm_queue_type type);
// Удалить очередь
void tm_queue_destroy(tm_queue_ctx *_q);

// Добавить в очередь массив блоков
int tm_queue_push_back(tm_queue_ctx *_q, client_block_t *client_block_array, int count);
// Забрать из очереди массив блоков
int tm_queue_pop_front(tm_queue_ctx *_q, client_block_t *client_block_array, int count);

// Проверить пуста ли очередь
int tm_queue_is_empty(tm_queue_ctx *_q);

// Получить строковое представление типа очереди
const char *tm_queue_type_to_str(tm_queue_type type);

#endif /* TM_QUEUE_H */
