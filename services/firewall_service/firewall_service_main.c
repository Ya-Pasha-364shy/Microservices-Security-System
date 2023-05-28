#include "../../include/services/firewall_service/firewall_service_main.h"

static unsigned int fw_keep_running = 1;
static fw_config_attrs_t fw_attrs;
static unsigned int net_bits, netmask, ip_bits, untrusted_counter;

static void fw_signal_handler(/* unused */int signum)
{
	MSS_PRINT_DEBUG("<%s>", __func__);
	MSS_PRINT_INFO("SIGINT signal handled, the \"FW\" program is terminated!");
	fw_keep_running = 0;
}

unsigned int fw_get_keep_running()
{	
	return fw_keep_running;
}

void firewall_service_err_handler(const char * message, void * data)
{
	perror(message);
	if (data && (sizeof(data) == sizeof(int *)))
	{
		close(*(int *)data);
	}

	exit(FW_SERVICE_SOCKET_CREATION_EXIT);
}

void fw_got_packet(u_char * args, const struct pcap_pkthdr * header, const u_char * packet)
{
	if (!fw_get_keep_running())
	{
		pthread_exit(NULL);
	}
	PTR_IS_NULL(args, pthread_exit(NULL));

	static unsigned int count;

	/* declare pointers to packet headers */
	const fw_sniff_ethernet * ethernet;  /* The ethernet header [1] */
	const fw_sniff_ip * ip;              /* The IP header */
	const fw_sniff_tcp * tcp;            /* The TCP header */
	const char * payload;                /* Packet payload */

	int size_ip;
	int size_tcp;
	int size_payload;

	MSS_PRINT_DEBUG("Packet number %d:", count+1);
	ip_bits = 0, count++;

	/* define ethernet header */
	ethernet = (fw_sniff_ethernet *)(packet);

	/* define/compute ip header offset */
	ip = (fw_sniff_ip *)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		MSS_PRINT_DEBUG("   * Invalid IP header length: %u bytes", size_ip);
		return;
	}

	/* print source and destination IP addresses */
	MSS_PRINT_DEBUG("       From: %s", inet_ntoa(ip->ip_src));
	MSS_PRINT_DEBUG("         To: %s", inet_ntoa(ip->ip_dst));

	ip_bits = ip->ip_src.s_addr;
	if ((ip_bits & netmask) == net_bits)
	{
		untrusted_counter++;
	}

	/* determine protocol */
	switch(ip->ip_p)
	{
		case IPPROTO_TCP:
			MSS_PRINT_DEBUG("   Protocol: TCP");
			break;
		case IPPROTO_UDP:
			MSS_PRINT_DEBUG("   Protocol: UDP");
			return;
		case IPPROTO_ICMP:
			MSS_PRINT_DEBUG("   Protocol: ICMP");
			return;
		case IPPROTO_IP:
			MSS_PRINT_DEBUG("   Protocol: IP");
			return;
		default:
			MSS_PRINT_DEBUG("   Protocol: unknown");
			return;
	}

	/* define/compute tcp header offset */
	tcp = (fw_sniff_tcp *)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp) * 4;
	if (size_tcp < 20)
	{
		MSS_PRINT_DEBUG("   * Invalid TCP header length: %u bytes", size_tcp);
		return;
	}

	MSS_PRINT_DEBUG("   Src port: %d", ntohs(tcp->th_sport));
	MSS_PRINT_DEBUG("   Dst port: %d", ntohs(tcp->th_dport));

	/* define/compute tcp payload (segment) offset */
	payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

	/* compute tcp payload (segment) size */
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

	/*
	 * Print payload data; it might be binary, so don't just
	 * treat it as a string.
	 */
	if (size_payload > 0)
	{
		MSS_PRINT_DEBUG("   Payload (%d bytes):", size_payload);
		fw_print_payload(payload, size_payload);
	}
	return;
}

void * fw_thread_start(void * thread_arg)
{
	MSS_PRINT_DEBUG("<%s>", __func__);
	int pthread_cancel_state;
	pcap_t * handle;
	char * filter_exp = fw_attrs.fw_attr_filter;
	struct bpf_program fp;
	bpf_u_int32 mask;
	bpf_u_int32 net;
	int num_packets = fw_attrs.fw_attr_pcts_num;
	char errbuf[PCAP_ERRBUF_SIZE];
	fw_thread_argument_t * ptarg = (fw_thread_argument_t *)thread_arg;
	
	pthread_cancel_state = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (pthread_cancel_state != FW_SERVICE_NORMAL_CODE)
	{
		MSS_PRINT_INFO("<%s:%d> unable to set cancel state of pthread %ld", __func__, __LINE__, pthread_self());
		pthread_exit(NULL);
	}
	if (pcap_lookupnet(ptarg->device_name, &net, &mask, errbuf) == FW_SERVICE_ERROR_CODE)
	{
		MSS_PRINT_INFO("Couldn't get netmask for device %s: %s",
		        ptarg->device_name, errbuf);
		pthread_cancel(pthread_self());
	}
	/* not using promiscuous mode */
	handle = pcap_open_live(ptarg->device_name, SNAP_LEN, 0, 1000, errbuf);
	if (handle == NULL)
	{
		MSS_PRINT_INFO("Couldn't open device %s: %s", ptarg->device_name, errbuf);
		pthread_cancel(pthread_self());
	}

	/* ethernet-link interfaces support only */
	if (pcap_datalink(handle) != DLT_EN10MB)
	{
		MSS_PRINT_INFO("%s is not an Ethernet", ptarg->device_name);
		pthread_cancel(pthread_self());
	}

	/* filter compiling */
	if (pcap_compile(handle, &fp, filter_exp, 1, net) == FW_SERVICE_ERROR_CODE)
	{
		MSS_PRINT_INFO("Couldn't parse filter %s: %s",
			filter_exp, pcap_geterr(handle));
		pthread_cancel(pthread_self());
	}

	/* filter apply */
	if (pcap_setfilter(handle, &fp) == -1)
	{
		MSS_PRINT_INFO("Couldn't install filter %s: %s",
			filter_exp, pcap_geterr(handle));
		pthread_cancel(pthread_self());
	}

	pthread_cancel_state = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &pthread_cancel_state);
	if (pthread_cancel_state != 0)
	{
		MSS_PRINT_INFO("<%s:%d> unable to set cancel state of pthread %ld", __func__, __LINE__, pthread_self());
		pthread_exit(NULL);
	}
	pcap_loop(handle, num_packets, fw_got_packet, (u_char *)ptarg);

	/* cleaning */
	pcap_freecode(&fp);
	pcap_close(handle);

	MSS_PRINT_INFO("thread %ld capture completed !", pthread_self())
	pthread_exit(NULL);
}

/*
 * @brief
 * main driver for "Firewalld" service
 * needed sniff packets and process them by iptables
*/
int start(int argc, char * argv[])
{
	uint8_t devices_num = 0;
	unsigned int sleep_now = 1, total_sleep;
	char errbuf[PCAP_ERRBUF_SIZE];
	pthread_mutex_t mutex;
	pcap_if_t * alldevs = NULL;
	fw_thread_argument_t * thread_args = NULL;

	if (NULL == freopen("/dev/null", "r", stdin)
#ifndef ENABLE_DEBUG_INFO
	    || NULL == freopen("/dev/null", "w", stderr)
	    || NULL == freopen("/dev/null", "w", stdout)
#endif
	)
	{
		MSS_PRINT_INFO("Failed to redirect I/Os to /dev/null, exiting.");
		return FW_SERVICE_ERROR_CODE;
	}

	if (signal(SIGINT, fw_signal_handler) == SIG_IGN)
	{
		/* Ignore the signal SIGINT (Ctrl+C) */
		signal(SIGINT, SIG_IGN);
	}
	if (FW_SERVICE_NORMAL_CODE != fw_parse_and_serialize_config(&fw_attrs))
	{
		MSS_PRINT_DEBUG("<%s:<%d> error for parsing config file", __func__, __LINE__);
		return FW_SERVICE_ERROR_CODE;
	}

	net_bits = fw_attrs.fw_untrusted_network.ip_prefix.s_addr;
	netmask  = net_bits & ((1 << fw_attrs.fw_untrusted_network.netmask) - 1);

	pcap_if_t * device = NULL;
	if (0 == strcmp(fw_attrs.fw_attr_interface, FW_ATTR_IFACE_ALL))
	{
		if (!pcap_findalldevs(&alldevs, errbuf))
		{
			for (device = alldevs; device; device = device->next, devices_num++);
		}
	}
	else
	{
		devices_num = FW_ONE_IFACE_SET;
	}
	if (!devices_num)
	{
		MSS_PRINT_INFO("No available devices detected! Exiting...");
		return FW_SERVICE_ERROR_CODE;

	}
	pthread_t threads[devices_num];
	if (FW_ONE_IFACE_SET != devices_num && 0 != pthread_mutex_init(&mutex, NULL))
	{
		MSS_PRINT_INFO("Error: pthread_mutex_init() failed !");
		pcap_freealldevs(alldevs);
		return FW_SERVICE_ERROR_CODE;
	}

	uint8_t i;
	char * device_name;
	thread_args = calloc(devices_num, sizeof(fw_thread_argument_t));
	for (device = alldevs, i = 0; device || i < devices_num; i++)
	{
		device_name = NULL;
		if (NULL == alldevs)
		{
			device_name = fw_attrs.fw_attr_interface;
			thread_args[i].common_mutex = NULL;
		}
		else if (device)
		{
			device = device->next;
			if (device)
			{
				device_name = device->name;
				thread_args[i].common_mutex = &mutex;
			}
			else	
			{
				break;
			}
		}
		else
		{
			break;
		}
		if (device_name)
		{
			strncpy(thread_args[i].device_name, device_name, strlen(device_name));
		}
	}

	int free_and_exit(int rc)
	{
		if (FW_ONE_IFACE_SET != devices_num)
		{
			pcap_freealldevs(alldevs);
			pthread_mutex_destroy(&mutex);
		}
		free(thread_args);
		int ipt_rc = fw_ip_tables_flush() < 0 || fw_ip_tables_save() < 0;

		return rc | ipt_rc;
	}

	fw_ip_tables_flush();
	if (fw_untrusted_network_rules_init() || fw_trusted_network_rules_init())
	{
		MSS_PRINT_DEBUG("<%s> error for creating chains for iptables !", __func__);
		return free_and_exit(FW_SERVICE_ERROR_CODE);
	}

	if (FW_SERVICE_NORMAL_CODE != fw_add_block_rule_for_untrusted_zone(&fw_attrs.fw_untrusted_network.ip_prefix,
										fw_attrs.fw_untrusted_network.netmask) ||
		FW_SERVICE_NORMAL_CODE != fw_add_accept_rule_for_trusted_zone(&fw_attrs.fw_trusted_network.ip_prefix,
										fw_attrs.fw_trusted_network.netmask, fw_attrs.fw_attr_interface))
	{
		MSS_PRINT_DEBUG("<%s:<%d> error for add rule in firewall", __func__, __LINE__);
		return free_and_exit(FW_SERVICE_ERROR_CODE);
	}
	fw_ip_tables_save();

	int rc;
	for (i = 0; i < devices_num && fw_keep_running; i++)
	{
		rc = pthread_create(&threads[i], NULL, &fw_thread_start, &thread_args[i]);
		if (FW_SERVICE_NORMAL_CODE != rc)
		{
			MSS_PRINT_INFO("<%s> Error for create thread id #%lu; return code: %d", __func__, threads[i], rc);
			return free_and_exit(FW_SERVICE_ERROR_CODE);
		}
	}

	for (i = 0; i < devices_num; i++)
	{
		while (fw_get_keep_running()) { sleep(sleep_now); continue; }

		total_sleep = 0;
		while (0 != pthread_tryjoin_np(threads[i], NULL))
		{
			total_sleep += sleep_now;
			sleep(sleep_now);

			if (!(total_sleep % FW_MAX_SEC_WAIT))
			{
				MSS_PRINT_INFO("!!! timeout for waiting %lu thread, exiting !!!", threads[i]);
				rc = FW_SERVICE_ERROR_CODE;
				break;
			}
			continue;
		}
		if (rc != 0)
		{
			MSS_PRINT_INFO("Error for join thread id #%lu; return code: %d", threads[i], rc);
		}
	}

	return free_and_exit(rc);
}

int main(int argc, char * argv[])
{
	int sockfd, n, i;
	struct sockaddr_in servaddr;

	if (argc != FW_SERVICE_UNEXPECTED_CODE)
	{
		firewall_service_err_handler("Usage: ./backup_service <IP address>", NULL);
	}

	if (FW_SERVICE_ERROR_CODE == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
	{
		firewall_service_err_handler("Can't create a socket with TCP type", NULL);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(7777);

	// connect to 0.0.0.0:7777 by default
	if (inet_aton(argv[1], &servaddr.sin_addr) == 0)
	{
		firewall_service_err_handler("Invalid IP address", &sockfd);
	}

	if (FW_SERVICE_ERROR_CODE == connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
	{
		firewall_service_err_handler("Impossible to connect with open socket", &sockfd);
	}

	service_message_t say_hello_firewall_service = {0}, answer = {0};
	say_hello_firewall_service.process_id = firewall_service_process_type;
	say_hello_firewall_service.signal = SIGNAL_FIREWALL_INIT;

	if ((n = write(sockfd, &say_hello_firewall_service, sizeof(say_hello_firewall_service))) < 0)
	{
		firewall_service_err_handler("Can't send data by socket", &sockfd);
	}
	if ((n = read(sockfd, &answer, sizeof(answer))) < 0)
	{
		firewall_service_err_handler("Can't read data from socket", &sockfd);
	}
	if (SIGNAL_MAKE_FIREWALL_ACCEPT == answer.signal)
	{
		if (FW_SERVICE_NORMAL_CODE != start(argc, argv))
		{
			firewall_service_err_handler("Error in the firewall-processing", &sockfd);
		}
	}
	close(sockfd);
	return FW_SERVICE_NORMAL_CODE;
}