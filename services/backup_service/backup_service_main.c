/*
 * \author Pavel Chernov (K1rch)
 * \brief  Service (process) for backup some directory
 *         which is specified in the configuration file
 *         named by backup-service
*/
#include "../../include/services/backup_service/backup_service_main.h"
#include "../../include/services/backup_service/helpers_backup_common.h"
#include "../../include/helpers/helpers_common.h"
#include "../../include/helpers/helpers_parser.h"

void backup_service_err_handler(const char * message, void * data)
{
	perror(message);
	if (data && (sizeof(data) == sizeof(int *)))
	{
		close(*(int *)data);
	}

	exit(BACKUP_SERVICE_SOCKET_CREATION_EXIT);
}

/* 
 * @brief
 * backup_loop
 * Need for "recursive backups"
*/
void * backup_loop(void * argument)
{
	MSS_PRINT_DEBUG("<%s> thread id %lu start running on backup loop successfully...",
	                __func__, pthread_self());

	backup_fs_iteration_main((thread_argument_t *)argument);
	
	pthread_exit(NULL);
}

void * general_loop(void * argument)
{
	MSS_PRINT_DEBUG("<%s>", __func__);
	return pthread_on_dir_run(argument);
}

/* TODO:
 * 1) change type of parsing conf file (from .yaml, for example)
*/
static int backup_parse_and_serialize_config(thread_argument_t * arg,
                                             char * path_to_dir_buffer)
{
	bool flag = true;
	int before = 0, after = 0, i = 0, j = 0, counter;
	size_t size_of_line;
	char * buffer, tmp, dir;
	char before_value[HELPERS_BUF_SIZE] = {0};

	buffer = parser_read_conf(BACKUP_PATH_TO_CONFIG);

	PTR_IS_NULL(buffer, INVALID_EXIT);
	while (flag)
	{
		for (i = before; buffer[i] != '\n'; i++)
		{
			if (buffer[i] == '\0')
			{
				flag = false;
			}
		}
		after = i;
		size_of_line = sizeof(char) * after;

		char tmp[after];

		counter = 0;
		for (j = before; j < after; j++)
		{
			tmp[counter++] = buffer[j];
		}
		tmp[counter] = '\0';

		if (strlen(tmp) > HELPERS_FILE_STROKE_MAX_LEN)
		{
			free(buffer);
			return INVALID_EXIT;
		}
		// skipping \n for normal work
		before = after + 1;
		int index = parser_get_index_by_param(tmp, PARSER_DELIMETER);
		if (0 > index)
		{
			// TODO: add check, that one of config required attrs is null
			break;
		}

		int k;
		// in result before_value is parameter without value
		for (k = 0; k != index; k++)
		{
			before_value[k] = tmp[k];
		}
		before_value[k] = '\0';

		// parse value (segment of data with some path)
		int local_counter = 0;
		char slice_after_delimeter[size_of_line];

		for (int i = index + 2; tmp[i] != '\0'; i++)
		{
			slice_after_delimeter[local_counter++] = tmp[i];
		}
		slice_after_delimeter[local_counter] = '\0';

		if (0 == strcmp(before_value, PATH_TO_DIR_DEFINE))
		{
			struct stat path_stat;
			stat(slice_after_delimeter, &path_stat);
			if (!S_ISDIR(path_stat.st_mode))
			{
				MSS_PRINT_DEBUG("Warning: this path is not a path to directory !");
				free(buffer);
				return INVALID_EXIT;
			}
			strncpy(path_to_dir_buffer, slice_after_delimeter, strlen(slice_after_delimeter));
		}
		else if (0 == strcmp(before_value, PATH_TO_BACKUP_DEFINE))
		{
			backup_set_path_to_backup(slice_after_delimeter);
		}
		else if (0 == strcmp(before_value, PATH_TO_LOGGER_DEFINE))
		{
			logger_set_path_to_log(slice_after_delimeter);
		}
		else
		{
			break;
		}
	}
	MSS_PRINT_INFO("The configuration of \"backup\" was successfully applied !");
	free(buffer);
	arg->path_to_dir = path_to_dir_buffer;

	return NORMAL_EXIT;
}

/*
 * @brief
 * main driver for "Backupd" service
 * needed to distribute the work with POSIX threads
*/
int start(int argc, char * argv[])
{
	const int sleep_now = SLEEP_ONE_SEC;
	void * (* func[THREAD_COUNT])(void * path_to_dir) = {backup_loop, general_loop};
	hash_table_t * HT = hash_table_create(HASH_TABLE_MAX);
	thread_argument_t * argument = (thread_argument_t *)calloc(1, sizeof(thread_argument_t));
	char path_to_dir_buffer[HELPERS_BUF_SIZE] = {0};
	argument->hash_table = HT;

	int free_and_exit(int rc)
	{
		hash_table_free(HT);
		free(argument);
		return rc;
	}

	if (NULL == freopen("/dev/null", "r", stdin)
#ifndef ENABLE_DEBUG_INFO
	    || NULL == freopen("/dev/null", "w", stderr)
	    || NULL == freopen("/dev/null", "w", stdout)
#endif
	)
	{
		MSS_PRINT_INFO("Failed to redirect I/Os to /dev/null, exiting.");
		return free_and_exit(INVALID_EXIT);
	}

	if (signal(SIGINT, backup_signal_handler) == SIG_IGN)
	{
		/* Ignore the signal SIGINT (Ctrl+C) */
		signal(SIGINT, SIG_IGN);
	}

	if (NORMAL_EXIT != backup_parse_and_serialize_config(argument, path_to_dir_buffer))
	{
		return free_and_exit(INVALID_EXIT);
	}

	pthread_t threads[THREAD_COUNT];
	pthread_mutex_t mutex;
	if (0 != pthread_mutex_init(&mutex, NULL))
	{
		MSS_PRINT_INFO("Error: pthread_mutex_init() failed !");
		return free_and_exit(INVALID_EXIT);
	}
	argument->mutex = &mutex;

	int i, rc = 0;
	for (i = 0; i < THREAD_COUNT; i++)
	{
		rc = pthread_create(&threads[i], NULL, func[i], argument);
		if (NORMAL_EXIT != rc)
		{
			MSS_PRINT_INFO("<%s> Error for create thread id #%lu; return code: %d", __func__, threads[i], rc);
			pthread_mutex_destroy(&mutex);
			return free_and_exit(INVALID_EXIT);
		}
	}

	for (i = 0; i < THREAD_COUNT; i++)
	{
		while (helpers_get_backup_keep_running()) { sleep(sleep_now); continue; }

		int total_sleep = 0;
		while (0 != pthread_tryjoin_np(threads[i], NULL))
		{
			total_sleep += sleep_now;
			sleep(sleep_now);

			if (!(total_sleep % BACKUP_MAX_SEC_WAIT))
			{
				MSS_PRINT_INFO("\n!!! timeout for waiting %lu thread, exiting !!!", threads[i]);
				rc = INVALID_EXIT;
				break;
			}
			continue;
		}
		if (rc != 0)
		{
			MSS_PRINT_INFO("Error for join thread id #%lu; return code: %d", threads[i], rc);
		}
	}

	pthread_mutex_destroy(&mutex);
	return free_and_exit(rc);
}

int main(int argc, char * argv[])
{
	int sockfd, n, i;
	struct sockaddr_in servaddr;

	if (argc != BACKUP_SERVICE_UNEXPECTED_CODE)
	{
		backup_service_err_handler("Usage: sudo ./firewall_service <IP address>", NULL);
	}

	if (BACKUP_SERVICE_ERROR_CODE == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
	{
		backup_service_err_handler("Can't create a socket with TCP type", NULL);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(7777);

	// default is 0.0.0.0:7777
	if (inet_aton(argv[1], &servaddr.sin_addr) == 0)
	{
		backup_service_err_handler("Invalid IP address", &sockfd);
	}

	if (BACKUP_SERVICE_ERROR_CODE == connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
	{
		backup_service_err_handler("Impossible to connect with open socket", &sockfd);
	}

	service_message_t say_hello_backup_service = {0}, answer = {0};
	say_hello_backup_service.process_id = backup_service_process_type;
	say_hello_backup_service.signal = SIGNAL_BACKUP_INIT;

	// Бэкап-сервис спрашивает разрешения от service-manager и начинает процесс запуска бэкапа.
	if ((n = write(sockfd, &say_hello_backup_service, sizeof(say_hello_backup_service))) < 0)
	{
		backup_service_err_handler("Can't send data by socket", &sockfd);
	}
	if ((n = read(sockfd, &answer, sizeof(answer))) < 0)
	{
		backup_service_err_handler("Can't read data from socket", &sockfd);
	}
	if (SIGNAL_MAKE_BACKUP_ACCEPT == answer.signal)
	{
		if (NORMAL_EXIT != start(argc, argv))
		{
			backup_service_err_handler("Error in the backup-processing", &sockfd);
		}
	}
	close(sockfd);
	return BACKUP_SERVICE_NORMAL_CODE;
}
