/*
 * \author Pavel Chernov (K1rch)
 * \brief  Service (process) for backup some directory
 *         which is specified in the configuration file
 *         named by backup-service
 *
 * \date   03.01.2023  
*/
#include "../../include/services/backup-service/backup-service-fs-iteration.h"
#include "../../include/helpers/helpers-common.h"
#include "../../include/helpers/helpers-message.h"

void backup_service_errno_handler(const char * message, void * data)
{
    perror(message);
    if (data && (sizeof(data) == sizeof(int *)))
    {
        close(*(int *)data);
    }

    exit(BACKUP_SERVICE_SOCKET_CREATION_EXIT);
}

int main(int argc, char * argv[])
{
    int sockfd;
    int n;
    int i;
    char recvline[MAX_BUFFER_SIZE];
    struct sockaddr_in servaddr;
    
    if (argc != BACKUP_SERVICE_UNEXPECTED_CODE)
    {
        backup_service_errno_handler("Usage: ./backup-service <IP address>", NULL);
    }
    bzero(recvline, MAX_BUFFER_SIZE);

    if (BACKUP_SERVICE_ERROR_CODE == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
    {
        backup_service_errno_handler("Can't create a socket with TCP type", NULL);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(7777);
    
    if (inet_aton(argv[1], &servaddr.sin_addr) == 0)
    {
        backup_service_errno_handler("Invalid IP address", &sockfd);
    }

    if (BACKUP_SERVICE_ERROR_CODE == connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
    {
        backup_service_errno_handler("Impossible to connect with open socket", &sockfd);
    }

    ipc_message_t say_hello_backup_service = {0}, answer = {0};
    say_hello_backup_service.process_id = backup_service_process_type;

    for (i = 0; i < 3; i++)
    {
        printf("String => ");
        fflush(stdin);
        if (NULL == fgets(say_hello_backup_service.message_data, MESSAGE_BUFFER_SIZE, stdin))
        {
            backup_service_errno_handler("Impossible to write data from stdin to sending buffer", &sockfd);
        }
        say_hello_backup_service.message_size = MESSAGE_BUFFER_SIZE;
        say_hello_backup_service.signal = SIGNAL_COMMON_SENDER; 

        if ((n = write(sockfd, &say_hello_backup_service, sizeof(say_hello_backup_service))) < 0)
        {
            backup_service_errno_handler("Can't send data by socket", &sockfd);
        }
        if ((n = read(sockfd, &answer, sizeof(answer))) < 0)
        {
            backup_service_errno_handler("Can't read data from socket", &sockfd);
        }
        MSS_PRINT_DEBUG("%s: ECHO REPLY = %s\n", __FUNCTION__, answer.message_data);

        if (SIGNAL_MAKE_BACKUP == answer.signal)
        {
            char * const * backup_argv = (char * const * )"/home/k1rch/CODE";
            if (BACKUP_SERVICE_ERROR_CODE == backup_service_fs_iteration_main(backup_argv))
            {
                backup_service_errno_handler("Error in make backup process", &sockfd);
            }
        }
    }
    close(sockfd);
    return BACKUP_SERVICE_NORMAL_CODE;
}