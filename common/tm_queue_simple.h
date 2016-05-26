/*
 * Реализация простой очереди на основе списка
 * */

#ifndef TM_QUEUE_SIMPLE_H
#define TM_QUEUE_SIMPLE_H

#include "tm_block.h"
#include "tm_alloc.h"

// Удалить очердь
void tm_queue_destroy_simple(void *_q);
// Создать очередь
void *tm_queue_create_simple();
// Добавить один элемент в очередь
int tm_queue_push_back_simple(void *_q, client_block_t *client_block);
// Забрать один элемент из очереди
client_block_t tm_queue_pop_front_simple(void *_q);
// Проверить очередь на пустоту
int tm_queue_is_empty_simple(void *_q);

#endif /*TM_QUEUE_SIMPLE_H*/