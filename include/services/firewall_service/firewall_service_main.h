#ifndef MSS_FW_MAIN_H
#define MSS_FW_MAIN_H

#include "helpers_firewall_common.h"
#include <pcap.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define FW_MAX_SEC_WAIT (30)

#define FW_ATTR_IFACE_ALL ("all")
#define FW_ONE_IFACE_SET  (1)

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN      (1518)

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET (14)

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN (6)

/* Ethernet header */
typedef struct sniff_ethernet
{
	u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
	u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
	u_short ether_type;                     /* IP? ARP? RARP? etc */
} fw_sniff_ethernet;

/* IP header */
typedef struct sniff_ip {
	u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
	u_char  ip_tos;                 /* type of service */
	u_short ip_len;                 /* total length */
	u_short ip_id;                  /* identification */
	u_short ip_off;                 /* fragment offset field */
#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* don't fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
	u_char  ip_ttl;                 /* time to live */
	u_char  ip_p;                   /* protocol */
	u_short ip_sum;                 /* checksum */
	struct in_addr ip_src, ip_dst;  /* source and dest address */
} fw_sniff_ip;

#define IP_HL(ip)    (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)     (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

typedef struct sniff_tcp {
	u_short th_sport;               /* source port */
	u_short th_dport;               /* destination port */
	tcp_seq th_seq;                 /* sequence number */
	tcp_seq th_ack;                 /* acknowledgement number */
	u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
	u_char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;                 /* window */
	u_short th_sum;                 /* checksum */
	u_short th_urp;                 /* urgent pointer */
} fw_sniff_tcp;

typedef struct thread_argument
{
#define FW_DEVICE_MAX_LEN 32
	char device_name[FW_DEVICE_MAX_LEN];
#undef FW_DEVICE_MAX_LEN
	// mutex - is common object for all threads
	pthread_mutex_t * common_mutex;
} fw_thread_argument_t;

unsigned int fw_get_keep_running(void);

void fw_got_packet(u_char * args, const struct pcap_pkthdr * header, const u_char * packet);

int main(int argc, char ** argv);

#endif