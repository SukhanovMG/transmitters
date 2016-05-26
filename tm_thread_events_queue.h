/*
 * Релазиация модуля потоков для событийно-ориентированной парадигмы с очередями
 * */

#ifndef TM_THREAD_EVENTS_QUEUE_H
#define TM_THREAD_EVENTS_QUEUE_H

#include "tm_thread.h"

// Инициализация модуля
TMThreadStatus tm_threads_init_events_queue(int count);
// Останов модуля
TMThreadStatus tm_threads_shutdown_events_queue();
// Запуск модуля в работу
TMThreadStatus tm_threads_work_events_queue();

#endif /* TM_THREAD_EVENTS_QUEUE_H */