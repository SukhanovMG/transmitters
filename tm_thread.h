/*
 * Общий модуль для работы с потоками
 * */

#ifndef TM_THREAD_H
#define TM_THREAD_H

// Коды возврата функций
typedef enum _TMThreadStatus {
	TMThreadStatus_SUCCESS = 0,
	TMThreadStatus_ERROR = 1
} TMThreadStatus;

// Тип реализации модуля потоков
typedef enum _TMThreadType {
	TMThreadType_Simple = 0,
	TMThreadType_Libev_queue,
	TMThreadType_Libev_pipe
} TMThreadType;

// Получить строковое представление типа модуля
const char * tm_threads_type_to_str(TMThreadType type);

// Инициализация модуля
TMThreadStatus tm_threads_init(int count);
// Останов работы модуля
TMThreadStatus tm_threads_shutdown();
// Запуск модуля в работу
TMThreadStatus tm_threads_work();

#endif /* TM_THREAD_H */
