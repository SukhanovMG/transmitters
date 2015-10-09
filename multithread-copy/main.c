
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <getopt.h>
#include "tm_read_config.h"
#include "tm_compat.h"
#include "tm_alloc.h"
#include "tm_thread.c"
#include "tm_logging.h"
#include "tm_block.h"

static char main_conf_file[PATH_MAX];	///< полный путь к файлу конфигурации приложения
static int clients_option_flag = 0;

//Вывод помощи по параметрам командной строки
static void main_show_help()
{
	printf("Usage: mt_copy [parameters]...\n");
	printf("\n");
	printf("Arguments:\n");
	printf("  -c PATH, --config=PATH     use specific configuration file\n");
	printf("  -C COUNT, --clients=COUNT  override clients count in configuration file\n");
	printf("  -h, --help                 show this help and exit\n");
}

/*
	Фукнция разбора полученных параметров приложения.
	argc количество аргументов
	argv массив аргументов
	0: все параметры успешно разобраны
	-1: произошла ошибка и надо завершить приложение
	1: надо завершить приложение (вывод версии или подсказки или неизвестный параметр)
*/
static int main_args_handle(int argc, char *argv[])
{
	struct stat buffer;
	int res = 0;
	int option_index = -1;
	const char *short_options = "c:C:h";
	const struct option long_options[] = {
			{
				"config",
				required_argument,
				NULL,
				'c' },
			{
				"clients",
				required_argument,
				NULL,
				'C' },
			{
				"help",
				no_argument,
				NULL,
				'h' }
	};

	memset(main_conf_file, 0, sizeof(main_conf_file));

	while ( ( res = getopt_long(argc, argv, short_options, long_options, &option_index) ) != -1 ) {
		switch(res){
			case 'c':
				tm_strlcpy(main_conf_file, optarg, sizeof(main_conf_file));
				break;
			case 'C':
				clients_option_flag = 1;
				tm_read_config_clients_count = atoi(optarg);
				break;
			case 'h':
			case '?':
			default:
				main_show_help();
				return 1;
		}
		option_index = -1;
	}

	/* неизвестно имя и расположение конфигурационного файла */
	if (stat(main_conf_file, &buffer)) {
		fprintf(stderr, "%s[%d]: configuration file (%s) failed\n", __FILE__, __LINE__, main_conf_file);
		return -1;
	}

	if (clients_option_flag)
	{
		if (tm_read_config_clients_count <= 0)
		{
			fprintf(stderr, "%s[%d]: bad clients count (%d)\n", __FILE__, __LINE__, tm_read_config_clients_count);
			return -1;
		}
	}

	return 0;
}


int main(int argc, char *argv[])
{
	int rc = EXIT_SUCCESS;
	int res;

	memset(main_conf_file, 0, sizeof(main_conf_file));

	/* разбор параметров командной строки приложения */
	if ((res = main_args_handle(argc, argv)) != 0) {
		if (res < 0) {
			rc = EXIT_FAILURE;
		}
		goto application_exit;
	}


	/* инициализация лога */
	if(tm_log_init("tm") != TMLogStatus_SUCCESS) {
		rc = EXIT_FAILURE;
		goto application_exit;
	}

	/* Инициализация и чтение конфигурационного файла */
	if (read_config_init(main_conf_file) != ReadConfigStatus_SUCCESS) {
		rc = EXIT_FAILURE;
		goto application_exit;
	}

	if (read_config() != ReadConfigStatus_SUCCESS) {
		rc = EXIT_FAILURE;
		goto application_exit;
	}

	/* Инициализация модуля работы с блоками */
	if (!tm_block_init()) {
		rc = EXIT_FAILURE;
		goto application_exit;
	}

	/* Инициализация рабочих потоков */
	if (tm_threads_init(configuration.clients_count) != TMThreadStatus_SUCCESS) {
		rc = EXIT_FAILURE;
		goto application_exit;
	}

	/* Запуск рабочих потоков */
	if (tm_threads_work() != TMThreadStatus_SUCCESS) {
		rc = EXIT_FAILURE;
		TM_LOG_TRACE("===Test failed.===");
	}
	else
	{
		TM_LOG_TRACE("===Test completed successfully.===");
	}

application_exit: {

	tm_threads_shutdown();

	tm_block_fin();

	/* удаление ресурсов конфигурации */
	read_config_destroy();

	/* завершение логирования */
	tm_log_destroy();

	return rc;
}
}
