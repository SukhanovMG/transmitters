#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE 256


typedef struct {
	size_t num_of_clients;
	size_t buffer_size;
	size_t bitrate;
} config_struct;

int read_config_parse(char *fname, config_struct* target_config_struct);