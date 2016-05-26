/*
 * Релазиация модуля потоков для событийно-ориентированной парадигмы с каналами
 * */

#ifndef TM_THREAD_EVENTS_H
#define TM_THREAD_EVENTS_H

#include "tm_thread.h"

// Инициализация модуля
TMThreadStatus tm_threads_init_events(int count);
// Останов модуля
TMThreadStatus tm_threads_shutdown_events();
// Запуск модуля в работу
TMThreadStatus tm_threads_work_events();

#endif /* TM_THREAD_EVENTS_H */
