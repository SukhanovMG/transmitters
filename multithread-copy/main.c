
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <getopt.h>
#include "tm_read_config.h"
#include "tm_compat.h"
#include "tm_alloc.h"
#include "tm_queue.h"

static char main_conf_file[PATH_MAX];	///< полный путь к файлу конфигурации приложения

void thread_function();

//Вывод помощи по параметрам командной строки
static void main_show_help()
{
	printf("Usage: mt_copy [parameters]...\n");
	printf("\n");
	printf("Arguments:\n");
	printf("  -c, --config=PATH    use specific configuration file\n");
	printf("  -h, --help           show this help and exit\n");
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
	const char *short_options = "c:h";
	const struct option long_options[] = {
			{
					"config",
					required_argument,
					NULL,
					'c' },
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

	return 0;
}


int main(int argc, char *argv[])
{
	int rc = EXIT_SUCCESS;
	int res, ret;

	memset(main_conf_file, 0, sizeof(main_conf_file));

	/* разбор параметров командной строки приложения */
	if ((res = main_args_handle(argc, argv)) != 0) {
		if (res < 0) {
			rc = EXIT_FAILURE;
		}
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

	printf("WorkThreadsCount = %d\n", configuration.work_threads_count);

	pthread_t *threads = tm_alloc(configuration.work_threads_count * sizeof(pthread_t));
	tm_queue_queue *queues = tm_alloc(configuration.work_threads_count * sizeof(tm_queue_queue));
	void *thread_status;


	for (int i = 0; i < configuration.work_threads_count; i++)
	{
		tm_queue_init(&queues[i]);
		pthread_create(&threads[i], NULL, (void*(*)(void *)) thread_function, &queues[i]);
	}


	for (int i = 0; i < configuration.work_threads_count; i++)
	{
		for (int j = 0; j < 10; j++)
			tm_queue_push_back(&queues[i]);
	}

	for (int i = 0; i < configuration.work_threads_count; i++)
	{
		pthread_join(threads[i], &thread_status);
		tm_queue_destroy(&queues[i]);
	}

	tm_free(queues);
	tm_free(threads);

application_exit: {

	printf("Application stoping\n");
	/* удаление ресурсов конфигурации */
	read_config_destroy();
	printf("Application stoped\n");

	return rc;
}
}

void thread_function(void* queue)
{
	void *block;
	printf("thread\n");
	for (int j = 0; j < 10; j++)
		block = tm_queue_pop_front(queue);
	tm_free(block);
}
