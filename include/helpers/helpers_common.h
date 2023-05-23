#ifndef HELPERS_COMMON_H
#define HELPERS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

#define MSS_BUFFER_SIZE 512

#define ENABLE_DEBUG_INFO

#define MSS_PRINT_DEBUG(fmt, ...) \
	fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n");

#define MSS_PRINT_INFO(fmt, ...) \
	fprintf(stdout, fmt, ##__VA_ARGS__); fprintf(stdout, "\n");

#define PTR_IS_NULL(pointer, ret_code) \
	if (pointer == NULL) \
	{ \
		MSS_PRINT_DEBUG("<%s:%d> %s is NULL !", __func__, __LINE__, #pointer); \
		return ret_code; \
	}

typedef enum
{
	SIGNAL_BACKUP_INIT,
	SIGNAL_MAKE_BACKUP_DENY,
	SIGNAL_MAKE_BACKUP_ACCEPT,
} mss_backup_service_signal_t;

typedef enum
{
	SIGNAL_FIREWALL_INIT,
	SIGNAL_MAKE_FIREWALL_DENY,
	SIGNAL_MAKE_FIREWALL_ACCEPT,
} mss_firewall_service_signal_t;

typedef enum
{
	backup_service_process_type,
	firewall_service_process_type,
} ipc_message_process_id;

typedef struct ipc_message_
{
	ipc_message_process_id  process_id;
	uint8_t signal;

	// for description of errors
	char message_data[MSS_BUFFER_SIZE];
} service_message_t;

void strinit(char * string);
void message_init(void * message, size_t * message_size);

#endif // HELPERS_COMMON_H
