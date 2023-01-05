#ifndef SERVICE_MANAGER_MAIN_H
#define SERVICE_MANAGER_MAIN_H

#include "../helpers/helpers-message.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CONNECTION_NUM 15

enum service_codes
{
    MGR_SERVICE_ERROR_CODE = -1,
    MGR_SERVICE_NORMAL_CODE,
    MGR_SERVICE_SOCKET_CREATION_EXIT,
    MGR_SERVICE_UNEXPECTED_CODE,
};

typedef struct {
    int sockfd;
    int newsockfd;
    ipc_message_t message;
} mgr_service_process_args_t;

static void service_child_process_handler(mgr_service_process_args_t* args);
static void service_errno_handler(const char * message, void * data, void * service_process_args);

#endif // SERVICE_MANAGER_MAIN_H