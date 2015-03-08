#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE 256


typedef struct {
	int num_of_clients;
} config_struct;

config_struct parse_config(char *fname);