#include "../../include/services/firewall_service/helpers_firewall_common.h"

#define UNTRUSTED_ZONE_RULES "UNTRUSTED"
#define TRUSTED_ZONE_RULES   "TRUSTED"

static void print_hex_ascii_line(const u_char *payload, int len, int offset)
{
	int i;
	int gap;
	const u_char *ch;

	/* offset */
	printf("%05d   ", offset);

	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");

	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");

	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");
	return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void fw_print_payload(const u_char *payload, int len)
{
	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}
	return;
}

int fw_parse_and_serialize_config(fw_config_attrs_t * fw_attrs)
{
	bool flag = true;
	int before = 0, after = 0, i = 0, j = 0, counter;
	size_t size_of_line;
	char * buffer, tmp, dir;
	char before_value[FW_BUF_SIZE] = {0};

	buffer = parser_read_conf(FW_PATH_TO_CONFIG);
	PTR_IS_NULL(buffer, FW_SERVICE_ERROR_CODE);

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

		if (strlen(tmp) > PARSER_FILE_STROKE_MAX_LEN)
		{
			free(buffer);
			return FW_SERVICE_ERROR_CODE;
		}
		// skipping \n for normal work
		before = after + 1;
		int index = parser_get_index_by_param(tmp, PARSER_DELIMETER);
		if (0 > index)
		{
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

		if (0 == strcmp(before_value, FW_INTERFACE_DEFINE))
		{
			strncpy(fw_attrs->fw_attr_interface, slice_after_delimeter, strlen(slice_after_delimeter));			
		}
		else if (0 == strcmp(before_value, FW_FILTER_DEFINE))
		{
			strncpy(fw_attrs->fw_attr_filter, slice_after_delimeter, strlen(slice_after_delimeter));
		}
		else if (0 == strcmp(before_value, FW_PACKET_NUM_DEFINE))
		{
			int rv = FW_SNIFF_INF_COUNT;
			if (0 != strncmp(FW_CONFIG_INF_PCKS, slice_after_delimeter, FW_CONFIG_INF_PCKS_LEN))
			{
				rv = atoi(slice_after_delimeter);
			}
			fw_attrs->fw_attr_pcts_num = rv;
		}
		else if (0 == strcmp(before_value, FW_UNTRUSTED_NETWORK_DEFINE) || 0 == strcmp(before_value, FW_TRUSTED_NETWORK_DEFINE))
		{
			char ip_prefix[FW_IP_PREFIX_MAXLEN] = {0};
			char * netmask = strchr(slice_after_delimeter, PARSER_SLASH);
			size_t ip_prefix_len = strlen(slice_after_delimeter) - strlen(netmask);

			strncpy(ip_prefix, slice_after_delimeter, ip_prefix_len);

			if (0 == strcmp(before_value, FW_TRUSTED_NETWORK_DEFINE))
			{
				fw_attrs->fw_trusted_network.netmask   = (uint32_t)atoi(netmask + sizeof(char));
				if (!inet_aton(ip_prefix, &fw_attrs->fw_trusted_network.ip_prefix))
				{
					MSS_PRINT_DEBUG("Unable to parse ip prefix for trusted zone. Exiting");
					exit(FW_SERVICE_ERROR_CODE);
				}
				MSS_PRINT_DEBUG("[t] result: %s/%u", inet_ntoa(fw_attrs->fw_trusted_network.ip_prefix),
				                          fw_attrs->fw_trusted_network.netmask);
			}
			else if (0 == strcmp(before_value, FW_UNTRUSTED_NETWORK_DEFINE))
			{
				fw_attrs->fw_untrusted_network.netmask = (uint32_t)atoi(netmask + sizeof(char));
				if (!inet_aton(ip_prefix, &fw_attrs->fw_untrusted_network.ip_prefix))
				{
					MSS_PRINT_DEBUG("Unable to parse ip prefix for untrusted zone. Exiting");
					exit(FW_SERVICE_ERROR_CODE);
				}
				MSS_PRINT_DEBUG("[u] result: %s/%u", inet_ntoa(fw_attrs->fw_untrusted_network.ip_prefix),
				                         fw_attrs->fw_untrusted_network.netmask);
			}
		}
		else
		{
			break;
		}
	}
	MSS_PRINT_INFO("The configuration of \"FW\" was successfully applied !");
	free(buffer);

	return FW_SERVICE_NORMAL_CODE;
}

int fw_ip_tables_save()
{
	return system("sudo iptables-save") < 0;
}

int fw_trusted_network_rules_init()
{
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables --new %s", TRUSTED_ZONE_RULES);

	return system(buffer) < 0;
}

int fw_trusted_network_rules_destroy()
{
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables --delete-chain %s", TRUSTED_ZONE_RULES);

	return system(buffer) < 0;
}

int fw_untrusted_network_rules_init()
{
	int rc;
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables --new %s", UNTRUSTED_ZONE_RULES);

	return system(buffer) < 0;
}

int fw_untrusted_network_rules_destroy()
{
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables --delete-chain %s", UNTRUSTED_ZONE_RULES);

	return system(buffer) < 0;
}

int fw_add_block_rule_for_untrusted_zone(struct in_addr * ip_prefix, uint32_t netmask)
{
	int rc, post_rc;
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables -A OUTPUT -d %s/%u -j %s",
	        inet_ntoa(*ip_prefix), netmask, UNTRUSTED_ZONE_RULES);

	rc = system(buffer) < 0;
	memset(buffer, 0, sizeof(buffer));	

	sprintf(buffer, "sudo iptables -A %s -j REJECT", UNTRUSTED_ZONE_RULES);

	post_rc = system(buffer) < 0;
	return rc | post_rc;
}

int fw_add_accept_rule_for_trusted_zone(struct in_addr * ip_prefix, uint32_t netmask, char * iface)
{
	int rc, post_rc;
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables -A INPUT -i %s -s %s/%u -j %s",
	        iface, inet_ntoa(*ip_prefix), netmask, TRUSTED_ZONE_RULES);

	rc = system(buffer) < 0;
	memset(buffer, 0, sizeof(buffer));	

	sprintf(buffer, "sudo iptables -A %s -j ACCEPT", TRUSTED_ZONE_RULES);

	post_rc = system(buffer) < 0;
	return rc | post_rc;
}

int fw_ip_tables_flush()
{
	char buffer[FW_BUF_SIZE] = {0};
	sprintf(buffer, "sudo iptables -F %s && sudo iptables -F %s && sudo iptables -F OUTPUT",
	        TRUSTED_ZONE_RULES, UNTRUSTED_ZONE_RULES);

	return (system(buffer) < 0 | fw_trusted_network_rules_destroy() | fw_untrusted_network_rules_destroy());
}