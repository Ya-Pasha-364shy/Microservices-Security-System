#ifndef BACKUP_SERVICE_MAIN_H
#define BACKUP_SERVICE_MAIN_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

enum backup_service_codes
{
    BACKUP_SERVICE_ERROR_CODE = -1,
    BACKUP_SERVICE_NORMAL_CODE,
    BACKUP_SERVICE_SOCKET_CREATION_EXIT,
    BACKUP_SERVICE_UNEXPECTED_CODE,
};

typedef struct {
    int sockfd;
    int newsockfd;
    char * line;
} backup_service_process_args_t;

void backup_service_errno_handler(const char * message, void * data);

#endif // BACKUP_SERVICE_MAIN_H