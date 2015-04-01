#ifndef READ_CONFIG_H_
#define READ_CONFIG_H_

#include <libconfig.h>
#include <inttypes.h>
#include <linux/limits.h>

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
		int work_threads_count; /*!< Количество рабочих потоков */
		char *config_file; /*!< Полный путь с именем файла конфигурации */
} read_config_parameters_t;

extern read_config_parameters_t configuration; /*!< Настройки программы */

ReadConfigStatus read_config_init(const char *config_file);
ReadConfigStatus read_config_destroy();
ReadConfigStatus read_config();

#endif /* READ_CONFIG_H_ */
