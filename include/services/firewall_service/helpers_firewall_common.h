#ifndef FW_HELPERS_H
#define FW_HELPERS_H

#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// /home/k1rch/CODE/Microservices-Security-System/include/services/firewall_service/helpers_firewall_common.h
// /home/k1rch/CODE/Microservices-Security-System/include/helpers/helpers_common.h
#include "../../helpers/helpers_common.h"
#include "../../helpers/helpers_parser.h"
#include "../backup_service/helpers_backup_common.h"

#define FW_INTERFACE_DEFINE  "interface"
#define FW_FILTER_DEFINE     "filter"
#define FW_PACKET_NUM_DEFINE "packet_num"
#define FW_PATH_TO_CONFIG    ("/services/firewall_service/config.conf")
#define FW_UNTRUSTED_NETWORK_DEFINE "untrusted_net"
#define FW_TRUSTED_NETWORK_DEFINE   "trusted_net"

#define FW_IP_PREFIX_MAXLEN (15)

#define FW_BUF_SIZE (512)
#define FW_SNIFF_INF_COUNT (-1)

#define FW_CONFIG_INF_PCKS "inf"
#define FW_CONFIG_INF_PCKS_LEN (3)

enum fw_service_codes
{
	FW_SERVICE_ERROR_CODE = -1,
	FW_SERVICE_NORMAL_CODE,
	FW_SERVICE_SOCKET_CREATION_EXIT,
	FW_SERVICE_UNEXPECTED_CODE,
};

typedef struct fw_network_s
{
	struct in_addr ip_prefix;
	uint32_t netmask;
} fw_network_t;

typedef struct config_attrs_s
{
#define FW_MIN_BUF_SIZE (16)
	char fw_attr_interface[FW_MIN_BUF_SIZE];
	char fw_attr_filter[FW_MIN_BUF_SIZE];
#undef FW_MIN_BUF_SIZE

	fw_network_t fw_untrusted_network; 
	fw_network_t fw_trusted_network;

	int fw_attr_pcts_num;
} fw_config_attrs_t;

void fw_print_payload(const u_char * payload, int len);
int fw_parse_and_serialize_config(fw_config_attrs_t * fw_attrs);

int fw_ip_tables_save();
int fw_ip_tables_flush();

int fw_trusted_network_rules_init();
int fw_trusted_network_rules_destroy();
int fw_add_accept_rule_for_trusted_zone(struct in_addr * ip_prefix, uint32_t netmask, char * iface);

int fw_untrusted_network_rules_init();
int fw_untrusted_network_rules_destroy();
int fw_add_block_rule_for_untrusted_zone(struct in_addr * ip_prefix, uint32_t netmask, char * iface);

#endif // FW_HELPERS_H