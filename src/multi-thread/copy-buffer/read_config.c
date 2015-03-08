#include "read_config.h"

void trim_spaces(char *s)
{
	char *end = &s[strlen(s) - 1];

	while ( (isspace(*end)) && end > s )
		end--;

	*(end + 1) = '\0';
}

config_struct parse_config(char *fname)
{
	FILE* config_file;
	char buf[MAX_LINE];
	char *token;
	config_struct cfg;

	cfg.num_of_clients = 1;

	config_file = fopen(fname, "r");
	if (config_file == NULL)
		return cfg;

	while(!feof(config_file))
	{
		if (fgets(buf, MAX_LINE, config_file))
		{
			char key[MAX_LINE], value[MAX_LINE];


			if (buf[0] == '\n' || buf[0] == '#')
				continue;

			token = strtok(buf, "=");
			if (token == NULL)
				continue;
			else
				strncpy(key, token, MAX_LINE);

			trim_spaces(key);

			token = strtok(NULL, "=");
			if (token == NULL)
				continue;
			else
				strncpy(value, token, MAX_LINE);
			trim_spaces(value);


			if (strcmp(key, "num_of_clients") == 0)
			{
				int buf = atoi(value);
				if (buf <= 0)
					continue;
				else
					cfg.num_of_clients = buf;
			}
		}
	}

	fclose(config_file);
	return cfg;
}
