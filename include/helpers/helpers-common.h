#ifndef HELPERS_COMMON_H
#define HELPERS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#define MAX_BUFFER_SIZE 4096

#define MSS_PRINT_INFO(fmt, value) \
        printf(fmt, value);

#define MSS_PRINT_DEBUG(fmt, value, ...) \
        fprintf(stderr, fmt, value, __VA_ARGS__);

typedef enum
{
    SIGNAL_COMMON_SENDER,
    SIGNAL_PARSE_CONFIGURATION,
    SIGNAL_MAKE_BACKUP,
    SIGNAL_TIME_SYNC,
} mss_signal;

void strinit(char * string);
void message_init(void * message, size_t * message_size);

#endif // HELPERS_COMMON_H