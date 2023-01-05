/*
 * \author Pavel Chernov (K1rch)
 * \brief  Server side for multiprocessing communication 
 *         named by service-manager
 *
 * \date   03.01.2023  
*/
#include "../include/services/service-manager-main.h"
#include "../include/helpers/helpers-common.h"
#include "../include/helpers/helpers-message.h"

/* TODO: make a logging. Replace perror, printf on smth log-func */

static void service_errno_handler(const char * message, void * data, void * service_process_args)
{
    perror(message);
    if (data && (sizeof(data) == sizeof(int *)))
    {
        close(*(int *)data);
    }
    if (service_process_args && (sizeof(service_process_args) == sizeof(mgr_service_process_args_t *)))
    {
        mgr_service_process_args_t * handler_data = (mgr_service_process_args_t *)service_process_args;
        close(handler_data->sockfd);
        close(handler_data->newsockfd);
        handler_data = NULL;
    }

    exit(MGR_SERVICE_SOCKET_CREATION_EXIT);
}

static void service_child_process_handler(mgr_service_process_args_t * args)
{
    pid_t pid;
    int childExitStatus;
    ssize_t n;
    pid = fork();

    if (MGR_SERVICE_ERROR_CODE == pid) 
    {
        service_errno_handler("Can't fork child process for message handler", NULL, args);
    }
    else if (MGR_SERVICE_NORMAL_CODE == pid)
    {
        /* child process is need for handle new connection (from some registered service) */
        while ((n = read(args->newsockfd, &args->message, sizeof(args->message))) > 0)
        {   
            if (backup_service_process_type == args->message.process_id &&
                SIGNAL_COMMON_SENDER == args->message.signal)
            {
                args->message.signal = SIGNAL_MAKE_BACKUP;
                if (MGR_SERVICE_ERROR_CODE == (n = write(args->newsockfd, &args->message,
                    sizeof(args->message))))
                {
                    service_errno_handler("Can't write an answer to client by newsocket", NULL, args);
                }
            }
            strinit(args->message.message_data);
        }
        exit(MGR_SERVICE_NORMAL_CODE);
    }
    else do
    {
        if (MGR_SERVICE_ERROR_CODE == (pid = waitpid(pid, &childExitStatus, WNOHANG)))
        {
            service_errno_handler("waitpid() error", NULL, NULL);
        }
        else if (MGR_SERVICE_NORMAL_CODE == pid)
        {
            /* 
             * child process is still running,
             * yet handle some connection with
             * service-manager and backup-service,
             * for example
            */
        }
        else
        {
            if (WIFEXITED(childExitStatus))
            {
                printf("Child exited with status of %d\n", WEXITSTATUS(childExitStatus));
            }
            else
            {
                printf("Child process didn't exit successfully: status code = %d\n", WEXITSTATUS(childExitStatus));
            }
            sleep(5);
        }
    }
    while (MGR_SERVICE_NORMAL_CODE == pid);

    return;
}

int main()
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < MGR_SERVICE_NORMAL_CODE)
    {
        service_errno_handler("Can't create a socket with TCP type", NULL, NULL);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(7777);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*
     * servaddr is set:
     * server ip       = 0.0.0.0
     * server port     = 7777
     * connection type = TCP
    */

    /* bind socket to 0.0.0.0:7777 with TCP connection */
    if (MGR_SERVICE_ERROR_CODE == bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        service_errno_handler("Can't bind socket to this data", &sockfd, NULL);
    }
    /* determine how many requests to the server can be processed together */
    if (MGR_SERVICE_ERROR_CODE == listen(sockfd, MAX_CONNECTION_NUM))
    {
        service_errno_handler("Impossible to listen to so many requests", &sockfd, NULL);
    }
    
    mgr_service_process_args_t args;
    while (true)
    {
        clilen = sizeof(cliaddr);
        if(MGR_SERVICE_ERROR_CODE == (newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)))
        {
            service_errno_handler("Can't accept connections on this socket", &sockfd, NULL);
        }

        args.sockfd = sockfd; args.newsockfd = newsockfd;
        service_child_process_handler(&args);
        close(newsockfd);
    }

    return MGR_SERVICE_NORMAL_CODE;
}