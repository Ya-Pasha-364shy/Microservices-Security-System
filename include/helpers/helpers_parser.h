#ifndef HELPERS_PARSER_H
#define HELPERS_PARSER_H

#include "helpers_common.h"

#define PARSER_NORMAL_CODE  (0)
#define PARSER_INVALID_EXIT (-1)

#define PARSER_DELIMETER           ':'
#define PARSER_SLASH               '/'
#define PARSER_FILE_STROKE_MAX_LEN (168)
#define PARSER_BUFFER_SIZE         (2048)

int parser_get_index_by_param(char * stroke, char param);
char * parser_read_conf(const char * path_to_config);

#endif // HELPERS_PARSER_H
