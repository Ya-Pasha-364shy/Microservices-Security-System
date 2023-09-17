#ifndef HELPERS_PARSER_H
#define HELPERS_PARSER_H

#include <stdint.h>
#include <arpa/inet.h>

#define SUPPORTED_INTERFACE_NAME_LENGTH (16)

/* Conditions for parsing json data and serialize */
typedef enum 
{
	CONDITION_INVALID = 0xFF,
	CONDITION_BACKUP_SERVICE = 0x00,
	CONDITION_FIREWALL_SERVICE
} parser_condition_t;

/* Configuration struct for backup service */
#define PATH_TO_DIR    "serviced directory"
#define PATH_TO_BACKUP "backup storage"
#define PATH_TO_LOG    "log file path"

typedef struct backup_service_attrs_s
{
	char path_to_directory[1024];
	char path_to_backup[1024];
	char path_to_logging[1024];
} backup_service_attrs_t;

/* Configuration structs for firewall service */
#define INTERFACE_ATTR      "interfaces"
#define FILTER_ATTR         "filter string"
#define UNTRUSTED_NET_ATTR  "untrusted network"
#define TRUSTED_NET_ATTR    "trusted network"
#define PACKET_NUMBERS_ATTR "packets number"

typedef struct fw_network_s
{
	struct in_addr ip_prefix;
	uint32_t netmask;
} fw_network_t;

#define SUPPORTED_INTERFACES_NUMBER (16)

typedef struct firewall_service_attrs_s
{
	char * fw_attr_interface[SUPPORTED_INTERFACES_NUMBER];
#define MAX_LENGTH_FILTER_STR (48)
	char fw_attr_filter[MAX_LENGTH_FILTER_STR];
#undef MAX_LENGTH_FILTER_STR

	fw_network_t fw_untrusted_network[SUPPORTED_INTERFACES_NUMBER];
	fw_network_t fw_trusted_network[SUPPORTED_INTERFACES_NUMBER];
	int fw_attr_pcts_num;
} firewall_service_attrs_t;

#endif // HELPERS_PARSER_H