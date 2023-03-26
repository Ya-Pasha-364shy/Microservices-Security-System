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
#include "helpers_backup_common.h"

#define ENABLE_DEBUG_INFO

#define SLEEP_ONE_SEC    (1)

#define NORMAL_EXIT      (0)
#define INVALID_EXIT     (-1)

#define THREAD_COUNT     (2)
#define MAIN_BUFFER_SIZE (128)

#define MAX_SEC_WAIT     (30)

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

void * backup_loop(void * argument);
void * main_loop(void * argument);
int main(int argc, char * argv[]);

#endif // BACKUP_SERVICE_MAIN_H