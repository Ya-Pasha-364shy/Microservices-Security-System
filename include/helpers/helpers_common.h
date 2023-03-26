#ifndef HELPERS_COMMON_H
#define HELPERS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#define MSS_BUFFER_SIZE 512

#define MSS_PRINT_INFO(fmt, value) \
        printf(fmt, value);

#define MSS_PRINT_DEBUG(fmt, value, ...) \
        fprintf(stderr, fmt, value, __VA_ARGS__);

typedef enum
{
	SIGNAL_BACKUP_INIT,
	SIGNAL_MAKE_BACKUP_DENY,
	SIGNAL_MAKE_BACKUP_ACCEPT,
} mss_backup_service_signal_t;

typedef enum
{
	parser_service_process_type,
	backup_service_process_type,
	time_service_process_type,
} ipc_message_process_id;

typedef struct ipc_message_
{
	ipc_message_process_id  process_id;
	mss_backup_service_signal_t signal;

	// for description of errors
	char message_data[MSS_BUFFER_SIZE];
} service_message_t;

void strinit(char * string);
void message_init(void * message, size_t * message_size);

#endif // HELPERS_COMMON_H