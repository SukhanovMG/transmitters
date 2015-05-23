#include "tm_read_config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "tm_alloc.h"
#include "tm_logging.h"

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

	if (!(configuration.config_file = (char*)tm_strdup(config_file, -1))) {
		fprintf(stderr, "%s[%d]: Allocate memrory error\n", __FILE__, __LINE__);
		return ReadConfigStatus_ERROR;
	}

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
		tm_free(configuration.config_file);
	memset(&configuration, 0, sizeof(configuration));

	return ReadConfigStatus_SUCCESS;
}

ReadConfigStatus read_config_compute_sleep_time()
{
	ReadConfigStatus ret = ReadConfigStatus_ERROR;

	struct timespec wait_time;
	double time_in_seconds = 0;

	if (configuration.block_size <= 0)
		return ret;

	if (configuration.bitrate <= 0)
		return ret;

	time_in_seconds = (double) configuration.block_size / (configuration.bitrate / 8 * 1024);
	wait_time.tv_sec = (int) time_in_seconds;
	wait_time.tv_nsec = (time_in_seconds - wait_time.tv_sec) * 1000000000;

	if (wait_time.tv_sec > 0 || wait_time.tv_nsec > 0)
	{
		configuration.sleep_time = wait_time;
		ret = ReadConfigStatus_SUCCESS;
	}

	return ret;
}
/**
 * @brief Чтение конфигурации
 * @return Transcoder_ReadConfigStatus_ERROR, TranscoderReadConfigStatus
 */
ReadConfigStatus read_config(void)
{

	if (!configuration_inited) {
		return ReadConfigStatus_ERROR;
	}

	/* Чтение конфигурационного файла */
	if (!config_read_file(&cfg, configuration.config_file)) {
		goto read_config_error;
	}

	if (!config_lookup_int(&cfg, "ClientsCount", &configuration.clients_count)) {
		TM_LOG_ERROR("Incomplete config file '%s'\n", configuration.config_file);
		goto read_config_error;
	}

	if (!config_lookup_int(&cfg, "BlockSize", &configuration.block_size)) {
		TM_LOG_ERROR("Incomplete config file '%s'\n", configuration.config_file);
		goto read_config_error;
	}

	if (!config_lookup_int(&cfg, "Bitrate", &configuration.bitrate)) {
		TM_LOG_ERROR("Incomplete config file '%s'\n", configuration.config_file);
		goto read_config_error;
	}

	if (read_config_compute_sleep_time() != ReadConfigStatus_SUCCESS) {
		TM_LOG_ERROR("Incompatible bitrate and/or block size");
		goto read_config_error;
	}


	return ReadConfigStatus_SUCCESS;

read_config_error:
	read_config_destroy();
	return ReadConfigStatus_ERROR;
}
