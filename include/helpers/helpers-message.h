#ifndef HELPERS_MESSAGE_H
#define HELPERS_MESSAGE_H

#include "helpers-common.h"

#define MESSAGE_BUFFER_SIZE 4096

typedef enum
{
    parser_service_process_type,
    backup_service_process_type,
    time_service_process_type,
} ipc_message_process_id;

typedef struct ipc_message_
{
    ipc_message_process_id process_id;
    mss_signal signal;
    char   message_data[MESSAGE_BUFFER_SIZE];
    size_t message_size;
} ipc_message_t;

#endif // HELPERS_MESSAGE_H