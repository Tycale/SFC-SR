
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include <unistd.h>

#include "service.h"


#ifndef SR6_SERVICE_CHAINING_PARSING_H
#define SR6_SERVICE_CHAINING_PARSING_H

#define IP6HDR_SIZE 40
#define IP6HDR_FRAG_SIZE 8
#define TCP_HASH_SIZE 36
#define TCP_MAX_SEG 0xFFFFFFFF

/*
 *	NextHeader field of IPv6 header
 */

#define NEXTHDR_HOP		    0	/* Hop-by-hop option header. */
#define NEXTHDR_TCP		    6	/* TCP segment. */
#define NEXTHDR_UDP		    17	/* UDP message. */
#define NEXTHDR_IPV6		41	/* IPv6 in IPv6 */
#define NEXTHDR_ROUTING		43	/* Routing header. */
#define NEXTHDR_FRAGMENT	44	/* Fragmentation/reassembly header. */
#define NEXTHDR_GRE		    47	/* GRE header. */
#define NEXTHDR_ESP		    50	/* Encapsulating security payload. */
#define NEXTHDR_AUTH		51	/* Authentication header. */
#define NEXTHDR_ICMP		58	/* ICMP for IPv6. */
#define NEXTHDR_NONE		59	/* No next header */
#define NEXTHDR_DEST		60	/* Destination options header. */
#define NEXTHDR_SCTP		132	/* SCTP message. */
#define NEXTHDR_MOBILITY	135	/* Mobility header. */

#define NEXTHDR_MAX		    255


/*
 * TCP flag
 */
#define FLAG_NS  0x100
#define FLAG_CWR 0x80
#define FLAG_ECE 0x40
#define FLAG_URG 0x20
#define FLAG_ACK 0x10
#define FLAG_PSH 0x8
#define FLAG_RST 0x4
#define FLAG_SYN 0x2
#define FLAG_FIN 0x1




struct __attribute__((__packed__)) parser_ipv6
{
    uint32_t version_class_flow;
    uint16_t length;
    uint8_t  next_header;
    uint8_t  hop_limit;
    struct in6_addr src;
    struct in6_addr dst;
};

struct __attribute__((__packed__)) parser_ipv6_sr
{
    uint8_t next_header;
    uint8_t hdr_length;
    uint8_t routing_type;
    uint8_t seg_left;
    uint8_t first_seg;
    uint16_t flags;
    uint8_t hmac_key;
};

struct __attribute__((__packed__)) parser_ipv6_frag
{
    uint8_t next_header;
    uint8_t reserved_1;
    uint8_t start_frag_offset;
    uint8_t end_frag_offset_res_m;
    uint32_t identification;
};

struct __attribute__((__packed__)) parser_tcp
{
    uint16_t src;
    uint16_t dst;
    uint32_t seq_nbr;
    uint32_t ack_nbr;
    uint16_t len_res_control;
    uint16_t win;
    uint16_t checksum;
    uint16_t urgent;
};

typedef void* (parse_t)(char *data, int *offset, int *next_header);
typedef int (nparse_t)(char *data, void **type, int offset, int *next_header);


parse_t parse_ipv6;
parse_t parse_ipv6_sr;
parse_t parse_ipv6_frag;
parse_t parse_tcp;

typedef void (show_t)(void *pkt);

show_t print_ipv6;
show_t print_ipv6_sr;
show_t print_tcp;
show_t print_ipv6_frag;

nparse_t *call_parse(int next_header);
show_t *call_print(int next_header);

#endif //SR6_SERVICE_CHAINING_PARSING_H
