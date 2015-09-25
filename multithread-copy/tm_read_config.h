#ifndef TM_READ_CONFIG_H_
#define TM_READ_CONFIG_H_

#include <libconfig.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <time.h>

/**
 * Перечислитель статусов чтения настроек
 */
typedef enum _ReadConfigStatus {
	ReadConfigStatus_ERROR = 0,/*!< Ошибки при чтении настроек */
	ReadConfigStatus_SUCCESS, /*!< Успешное чтение настроек */
} ReadConfigStatus;

/**
 * Настройки программы
 */
typedef struct _read_config_parameters_t {
		char *config_file; /*!< Полный путь с именем файла конфигурации */
		int clients_count; /*!< Количество рабочих потоков */
		int block_size; /*!< Размер передаваемого блока */
		int bitrate; /*!< Битрейт информации как бы поступающей на вход */
		struct timespec sleep_time; /*!< Период между распределением пакетов по потокам */
		int avg_bitrate_calc_time; /*!< Время (в секундах), в течении которого накапливается информация о битрейте перед его рассчётом */
		int bitrate_diff_percent; /*!< Разница в процентах от исходного битрейта, которая может быть у клиента */
		double bitrate_diff; /*!< Разница в единицах битрейта от исходного битрейта, которая может быть у клиента */
		int test_time; /*!< Время в секундах, в течении которого проводится тест, т.е. работает программа. Если битрейт это время был достаточен, то программа успешно завершается */
		int mempool_use; /* Флаг использования пула */
} read_config_parameters_t;

extern read_config_parameters_t configuration; /*!< Настройки программы */
extern int tm_read_config_clients_count;

ReadConfigStatus read_config_init(const char *config_file);
ReadConfigStatus read_config_destroy();
ReadConfigStatus read_config();

#endif /* TM_READ_CONFIG_H_ */
