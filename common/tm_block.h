/*
 * Модуль для работы с блоками
 * */

#ifndef TM_BLOCK_H
#define TM_BLOCK_H

#include <stddef.h>
#include "tm_refcount.h"

// Структура блока
typedef struct _tm_block {
	tm_refcount refcount; // Счётчик ссылок
	char data[];          // Содержимое блока, данные
} tm_block;

// Структура для передачи блока внутри программы
typedef struct {
	tm_block *block;  // указатель на блок
	size_t client_id; // id клиента, для которого предназначен блок
} client_block_t;

// Инициализация модуля
int tm_block_init();
// Освобождение ресурсов модуля
void tm_block_fin();

// Удалить блок (освобождение или возврат в пул)
void tm_block_destroy(tm_block *block);
// Деструктор для блока, вызывает функцию удаления блока, исп-ся в счётчике ссылок
void tm_block_destructor(void *obj);
// Создать блок
tm_block *tm_block_create();
// Создать копию блока
tm_block *tm_block_copy(tm_block *block);
// Передать блок (либо копирование, либо проброс указателя с инкрементацией счётчика ссылок)
tm_block *tm_block_transfer_block(tm_block *block);
// Избавиться от блока (декрементирование счётчика ссылок и если надо удаление)
void tm_block_dispose_block(tm_block *block);

#endif /* TM_BLOCK_H */
