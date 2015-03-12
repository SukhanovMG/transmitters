#include "read_config.h"

static void trim_spaces(char *s)
{
	char *start = s;
	char *end = &s[strlen(s) - 1];

	// удаляем пробелы справа
	while ( (isspace(*end)) && end > s )
		end--;

	*(end + 1) = '\0';

	// слева
	while ( (isspace(*start)) && start < end )
		start++;

	// и копируем в начало буфера
	strcpy(s, start);	
}


int config_parse(char *fname, config_struct *target_config_struct)
{
	FILE* config_file;
	char buf[MAX_LINE];
	char *token;
	config_struct cfg;

	if (target_config_struct == NULL)
		return -1;

	// стандартные значения, на всякий случай
	cfg.num_of_clients = 1;
	cfg.buffer_size = 1024;
	cfg.bitrate = 16;

	config_file = fopen(fname, "r");
	if (config_file == NULL)
		return -1;

	while(!feof(config_file))
	{
		if (fgets(buf, MAX_LINE, config_file))
		{
			char key[MAX_LINE], value[MAX_LINE];

			// комментарии и пустые строки
			if (buf[0] == '\n' || buf[0] == '#')
				continue;

			// получаем строку до знака =
			token = strtok(buf, "=");
			if (token == NULL)
				continue;
			else
				strncpy(key, token, MAX_LINE);
			// обрезаем ей пробелы
			trim_spaces(key);

			// и после знака равно
			token = strtok(NULL, "=");
			if (token == NULL)
				continue;
			else
				strncpy(value, token, MAX_LINE);
			trim_spaces(value);

			// параметр "число клиентов"
			if (strcmp(key, "num_of_clients") == 0)
			{
				size_t val = (size_t) atoi(value);
				if (val <= 0)
					continue;
				else
					cfg.num_of_clients = val;
			}

			// параметр "размер буфера"
			if (strcmp(key, "buffer_size") == 0)
			{
				size_t val = (size_t) atoi(value);
				if (val <= 0)
					continue;
				else
					cfg.buffer_size = val;
			}


			// параметр "битрейт"
			if (strcmp(key, "bitrate") == 0)
			{
				size_t val = (size_t) atoi(value);
				if (val <= 0)
					continue;
				else
					cfg.buffer_size = val;
			}

		}
	}

	fclose(config_file);

	*(target_config_struct) = cfg;

	return 0;
}

