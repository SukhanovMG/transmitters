#include "read_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void thread_function();

int main(int argc, char *argv[])
{
	config_struct cfg;
	pthread_t *thread_mas;

	cfg = parse_config("conf");
	printf("num_of_clients = %d\n", cfg.num_of_clients);

	thread_mas = (pthread_t *) malloc(cfg.num_of_clients * sizeof(pthread_t));

	for(int i = 0; i < cfg.num_of_clients; i++)
	{
		pthread_create(&thread_mas[i], NULL, (void *(*)(void*)) thread_function, NULL);
	}

	for(int i = 0; i < cfg.num_of_clients; i++)
	{
		pthread_join(thread_mas[i], NULL);
	}

	free(thread_mas);

	return EXIT_SUCCESS;
}

void thread_function()
{
	printf("Thread created\n");
}