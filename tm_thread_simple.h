/**
 * Реализация простого модуля потоков (императивная парадигма программирования)
 */

#ifndef TM_THREAD_SIMPLE_H
#define TM_THREAD_SIMPLE_H

#include "tm_thread.h"

// Инициализация модуля
TMThreadStatus tm_threads_init_simple(int count);
// Останов модуля
TMThreadStatus tm_threads_shutdown_simple();
// Запуск модуля в работу
TMThreadStatus tm_threads_work_simple();

#endif /* TM_THREAD_SIMPLE_H */
