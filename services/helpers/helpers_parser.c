#include "../../include/helpers/helpers_parser.h"

int parser_get_index_by_param(char * stroke, char param)
{
	MSS_PRINT_DEBUG("<%s>", __func__);
	PTR_IS_NULL(stroke, PARSER_INVALID_EXIT);

	if (strlen(stroke) > PARSER_FILE_STROKE_MAX_LEN)
		return -2;

	char * substr_byparam = strchr(stroke, param);
	PTR_IS_NULL(substr_byparam, PARSER_INVALID_EXIT);

	int res = substr_byparam - stroke;
	return res;
}

char * parser_read_conf(const char * path_to_config)
{
	MSS_PRINT_DEBUG("<%s>", __func__);
	char cwd[PARSER_BUFFER_SIZE] = {0};
	PTR_IS_NULL(getcwd(cwd, sizeof(cwd)), NULL);
	char * pathToConf = (char *)calloc(strlen(cwd) + strlen(path_to_config) + 1, sizeof(char));

	strcat(pathToConf, cwd);
	strcat(pathToConf, path_to_config);

	FILE * cp = fopen(pathToConf, "r");
	if (NULL == cp)
	{
		MSS_PRINT_DEBUG("Error for opening file!");
		free(pathToConf);
		return NULL;
	}

	int lines;
	while (!feof(cp))
	{
		if (fgetc(cp) == '\n')
			lines++;
	}
	lines++;
	fclose(cp);

	char * buffer = (char *)malloc(sizeof(char) * PARSER_FILE_STROKE_MAX_LEN * lines);
	memset(buffer, 0, sizeof(char) * PARSER_FILE_STROKE_MAX_LEN * lines);
	if (NULL == (cp = fopen(pathToConf, "r")))
	{
		free(pathToConf);
		free(buffer);
		return NULL;
	}

	free(pathToConf);
	while (fread(buffer, sizeof(char), PARSER_FILE_STROKE_MAX_LEN * lines, cp));

	return buffer;
}