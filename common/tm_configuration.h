#ifndef TM_CONFIGURATION_H_
#define TM_CONFIGURATION_H_

#include "tm_queue.h"
#include "../tm_thread.h"

#include <libconfig.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <unistd.h>
#include <time.h>

/**
 * Перечислитель статусов чтения настроек
 */
typedef enum _ConfigurationStatus {
	ConfigurationStatus_ERROR = 0,/*!< Ошибки при чтении настроек */
	ConfigurationStatus_SUCCESS, /*!< Успешное чтение настроек */
} ConfigurationStatus;

/**
 * Настройки программы
 */
typedef struct _read_config_parameters_t {
	char *config_file;                 /*!< Полный путь с именем файла конфигурации */
	int work_threads_count;            /*!< Число рабочих потоков */
	int clients_count;                 /*!< Количество рабочих потоков */
	int block_size;                    /*!< Размер передаваемого блока */
	int bitrate;                       /*!< Битрейт информации как бы поступающей на вход */
	useconds_t sleep_time;             /*!< Период между распределением пакетов по потокам */
	int avg_bitrate_calc_time;         /*!< Время (в секундах), в течении которого накапливается информация о битрейте перед его рассчётом */
	int bitrate_diff_percent;          /*!< Разница в процентах от исходного битрейта, которая может быть у клиента */
	double bitrate_diff;               /*!< Разница в единицах битрейта от исходного битрейта, которая может быть у клиента */
	int test_time;                     /*!< Время в секундах, в течении которого проводится тест, т.е. работает программа. Если битрейт это время был достаточен, то программа успешно завершается */
	int use_mempool;                   /*!< Флаг использования пула */
	int copy_block_on_transfer;        /*!< Флаг копирования блока при передаче от главного потока к рабочему */
	TMThreadType thread_module_type;   /*!< Тип модуля потоков */
	int return_pointers_through_pipes; /*!< Флаг возвращение блоков на главный поток для работы со счётчиком ссылок */
	int optimize_refcount_use_by_copy; /*!< Делать по одной копии входящего блока для каждого рабочего потока */
	int refcount_with_mutex;           /*!< Использовать мьютекс в счётчике ссылок */
	tm_queue_type queue_type;          /*!< Тип очереди */
} read_config_parameters_t;

extern read_config_parameters_t configuration; /*!< Настройки программы */

ConfigurationStatus tm_configuration_init(const char *config_file, int clients_count, int work_threads_count);
ConfigurationStatus tm_configuration_configure();
ConfigurationStatus tm_configuration_destroy();

#endif /* TM_CONFIGURATION_H_ */
