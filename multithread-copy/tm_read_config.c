#include "read_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


read_config_parameters_t configuration;
static config_t cfg; /*!< Котекст библиотеки чтения файлов конфигурации */
static int configuration_inited = 0; /*!< флаг инициализированности конфигурации */

/**
 * Инициализация конфигурации
 * @param config_file путь и имя файла конфигурации
 * @return ReadConfigStatus_ERROR, TranscoderReadConfigStatus_SUCCESS
 */
ReadConfigStatus read_config_init(const char *config_file)
{
	if (configuration_inited)
		return ReadConfigStatus_SUCCESS;

	memset(&configuration, 0, sizeof(configuration));
	strncpy(configuration.config_file, config_file, strlen(config_file));
	config_init(&cfg);
	configuration_inited = 1;
	return ReadConfigStatus_SUCCESS;
}

/**
 * Уничтожение ресурсов занятых конфигурационными данными
 * @return Transcoder_ReadConfigStatus_ERROR, TranscoderReadConfigStatus
 */
ReadConfigStatus read_config_destroy(void)
{
	if (!configuration_inited)
		return ReadConfigStatus_SUCCESS;

	configuration_inited = 0;
	config_destroy(&cfg);

	/* Освобождение ресурсов */
	if (configuration.config_file)
		free(configuration.config_file);
	memset(&configuration, 0, sizeof(configuration));
}

/**
 * @brief Чтение конфигурации
 * @return Transcoder_ReadConfigStatus_ERROR, TranscoderReadConfigStatus
 */
ReadConfigStatus read_config(void)
{
	const char *path;
	struct stat st;

	if (!configuration_inited) {
		return ReadConfigStatus_ERROR;
	}

	/* Чтение конфигурационного файла */
	if (!config_read_file(&cfg, configuration.config_file)) {
		goto read_config_error;
	}

	if (!config_lookup_int(&cfg, "WorkThreadsCount", &configuration.work_threads_count)) {
		goto read_config_error;
	}


	return ReadConfigStatus_SUCCESS;

read_config_error:
	read_config_destroy();
	return ReadConfigStatus_ERROR;
}
