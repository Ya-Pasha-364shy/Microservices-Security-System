/*
 * \author Pavel Chernov (K1rch)
 * \brief  Server side for multiprocessing communication
 *         named by service-manager
 *
 * \date   03.01.2023
*/
#include "../include/services/service_manager_main.h"

static void service_err_handler(const char * message, void * data, void * service_process_args)
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
	int status_of_child_proc;
	pid_t pid, cchild;
	ssize_t n;

	pid = fork();
	if (MGR_SERVICE_ERROR_CODE == pid)
	{
		service_err_handler("Can't fork child process for message handler", NULL, args);
	}
	else if (MGR_SERVICE_NORMAL_CODE == pid)
	{
		/* ready to create another child of the child with `pid` */
		/* child process is need for handle new connection (from some registered service) */
		cchild = fork();
		if (MGR_SERVICE_ERROR_CODE == cchild)
		{
			service_err_handler("Can't fork child process of child process for message handler", NULL, args);
		}
		else if (MGR_SERVICE_NORMAL_CODE == cchild)
		{
			while ((n = read(args->newsockfd, &args->message, sizeof(args->message))) > 0)
			{
				if (backup_service_process_type == args->message.process_id &&
				    SIGNAL_BACKUP_INIT == args->message.signal)
				{
					args->message.signal = SIGNAL_MAKE_BACKUP_ACCEPT;
					if (MGR_SERVICE_ERROR_CODE == (n = write(args->newsockfd, &args->message,
					    sizeof(args->message))))
					{
						args->message.signal = SIGNAL_MAKE_BACKUP_DENY;
						service_err_handler("Can't write an answer to client by newsocket", NULL, args);
					}
				}
			}
			exit(MGR_SERVICE_NORMAL_CODE);
		}
		else
		{
			if ((cchild = wait(&status_of_child_proc)) == MGR_SERVICE_ERROR_CODE)
			{
				service_err_handler("unable to waiting child process", NULL, NULL);
			}
			if (WIFEXITED(status_of_child_proc))
			{
				MSS_PRINT_INFO("child exited with status of %d", WEXITSTATUS(status_of_child_proc));
			}
			else if (WIFSIGNALED(status_of_child_proc))
			{
				MSS_PRINT_INFO("child was terminated by signal %d", WTERMSIG(status_of_child_proc));
			}
			else if (WIFSTOPPED(status_of_child_proc))
			{
				MSS_PRINT_INFO("child was stopped by signal %d", WSTOPSIG(status_of_child_proc));
			}
			else
			{
				MSS_PRINT_INFO("reason unknown for child termination");
			}
		}
		exit(MGR_SERVICE_NORMAL_CODE);
	}

	return;
}

int main()
{
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in servaddr, cliaddr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < MGR_SERVICE_NORMAL_CODE)
	{
		service_err_handler("Can't create a socket with TCP type", NULL, NULL);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons(7777);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*
	 * servaddr is set:
	 * server ip -       0.0.0.0
	 * server port-      7777
	 * connection type - TCP
	 * 
	 * P.S.: set not default servers ip in production
	*/

	/* bind socket to 0.0.0.0:7777 with TCP connection */
	if (MGR_SERVICE_ERROR_CODE == bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
	{
		service_err_handler("Can't bind socket to this data", &sockfd, NULL);
	}
	/* determine how many requests to the server can be processed together */
	if (MGR_SERVICE_ERROR_CODE == listen(sockfd, MAX_CONNECTION_NUM))
	{
		service_err_handler("Impossible to listen to so many requests", &sockfd, NULL);
	}

	mgr_service_process_args_t args;
	while (true)
	{
		clilen = sizeof(cliaddr);
		if(MGR_SERVICE_ERROR_CODE == (newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)))
		{
			service_err_handler("Can't accept connections on this socket", &sockfd, NULL);
		}
		args.sockfd = sockfd; args.newsockfd = newsockfd;
		service_child_process_handler(&args);

		close(newsockfd);
	}

	return MGR_SERVICE_NORMAL_CODE;
}