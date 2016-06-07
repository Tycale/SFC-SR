#include "parsing.h"

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>


#include <unistd.h>

int find_hdr(int limit, char *pkt, void **res, int *nh) {
    int offset = 0;

    if((offset = parse_until(limit, pkt, offset, nh)) < 0)
        return -1;

    call_parse(*nh)(pkt, res, 0, nh);

    if(res != NULL)
        *res = pkt+offset;

    return offset;
}

int find_hdr_before(char *pkt, int nh_limit, int nh_before, void **limit, void **before, int nh) {
    int off_limit, off_before=-1, off_tmp=0, next_header = nh;
    nparse_t *parse;
    void *tmp;

    if((off_limit = parse_until(nh_limit, pkt, 0, &next_header)) == -1)
        return -1;

    while( off_limit >= 0 && (parse = call_parse(nh)) != NULL && off_tmp < off_limit) {
        if(nh == nh_before)
            off_before = off_tmp;

        off_tmp += parse(pkt, &tmp, off_tmp, &nh);
    }


    if(off_limit < 0 || off_before < 0)
        return -1;

    if(limit != NULL)
        *limit = pkt + off_limit;
    if(before != NULL)
        *before = pkt + off_before;

    return 0;
}


int find_hdr_before_compute(char *pkt, int nh_limit, int nh_before, void **limit, void **before, int nh, unsigned int *hash) {
    int off_limit, off_before=-1, off_tmp=0, next_header = nh;
    nparse_t *parse;
    void *tmp;

    if((off_limit = parse_until(nh_limit, pkt, 0, &next_header)) == -1)
        return -1;

    while( off_limit >= 0 && (parse = call_parse(nh)) != NULL && off_tmp < off_limit) {
        if(nh == nh_before)
            off_before = off_tmp;

        if(nh == NEXTHDR_IPV6){
            *hash = csum_partial(pkt+off_tmp+8, 32, *hash);
            //hexdump(pkt+off_tmp+8, 32);
        }

        off_tmp += parse(pkt, &tmp, off_tmp, &nh);
    }


    if(off_limit < 0 || off_before < 0)
        return -1;

    if(limit != NULL)
        *limit = pkt + off_limit;
    if(before != NULL)
        *before = pkt + off_before;

    return 0;
}

int parse_until(int limit, char *pkt, int offset, int *next_header){
    int parsed = 0;
    nparse_t *parse;
    void *tmp;

    while((parse = call_parse(*next_header)) != NULL && *next_header != limit)
        parsed += parse(pkt, &tmp, offset+parsed, next_header);

    if(parse == NULL)
        return -1;

    return parsed;
}

int parse_to(int limit, char *pkt, int offset, int *next_header){
    int parsed = 0;
    nparse_t *parse;
    void *tmp;

    while((parse = call_parse(*next_header)) != NULL && *next_header != limit)
        parsed += parse(pkt, &tmp, offset+parsed, next_header);


    if(parse != NULL)
        parsed += parse(pkt, &tmp, offset+parsed, next_header);
    else
        return -1;

    return parsed;
}

int nparse_ipv6(char *data, void **hdr, int offset, int *next_header){
    char str_src[INET6_ADDRSTRLEN];
    char str_dst[INET6_ADDRSTRLEN];

    struct parser_ipv6 **ip6hdr =  (struct parser_ipv6 **) hdr;
    *ip6hdr = (struct parser_ipv6 *) (data + offset);

    if( ! inet_ntop(AF_INET6, &((*ip6hdr)->src), str_src, INET6_ADDRSTRLEN) ) {
        fprintf(stderr, "inet_ntop error with source IPv6 parsing\n");
        return -1;
    }

    if( ! inet_ntop(AF_INET6, &((*ip6hdr)->dst), str_dst, INET6_ADDRSTRLEN)) {
        fprintf(stderr, "inet_ntop error with destination IPv6 parsing\n");
        return -1;
    }

    if(next_header != NULL)
        *next_header = (*ip6hdr)->next_header;

    return 40;
}

int nparse_ipv6_sr(char *data, void **hdr, int offset, int *next_header) {
    struct parser_ipv6_sr **ip6_sr_hdr =  (struct parser_ipv6_sr **) hdr;
    *ip6_sr_hdr = (struct parser_ipv6_sr *) (data + offset);

    if(next_header != NULL)
        *next_header = (*ip6_sr_hdr)->next_header;

    return 8 + 8 * (*ip6_sr_hdr)->hdr_length;
}

int nparse_ipv6_frag(char *data, void **hdr, int offset, int *next_header){
    struct parser_ipv6_frag **ip6_frag_hdr = (struct parser_ipv6_frag **) hdr;
    *ip6_frag_hdr = (struct parser_ipv6_frag *) (data + offset);

    if(next_header != NULL)
        *next_header = (*ip6_frag_hdr)->next_header;

    return 8;
}

int nparse_tcp(char *data, void **hdr, int offset, int *next_header) {
    struct parser_tcp *tcp_hdr = (struct parser_tcp *) (data + offset);

    if(next_header != NULL)
        *next_header = -1;

    if(hdr != NULL)
        *hdr = (void *) (data+offset);

    return 4 * ((ntohs(tcp_hdr->len_res_control) >> 12) & 0xf);
}

nparse_t *call_parse(int next_header){
    switch (next_header){
        case NEXTHDR_ROUTING:
            return &nparse_ipv6_sr;
        case NEXTHDR_IPV6:
            return &nparse_ipv6;
        case NEXTHDR_TCP:
            return &nparse_tcp;
        case NEXTHDR_UDP:
            break;
        case NEXTHDR_ICMP:
            break;
        case NEXTHDR_FRAGMENT:
            return &nparse_ipv6_frag;
        default:
            return NULL;
            //printf("Next header unknow or end-state : %d\n", next_header);
    }
    return NULL;
}

show_t *call_print(int next_header){
    switch (next_header){
        case NEXTHDR_ROUTING:
            return &print_ipv6_sr;
        case NEXTHDR_IPV6:
            return &print_ipv6;
        case NEXTHDR_TCP:
            return &print_tcp;
        case NEXTHDR_UDP:
            break;
        case NEXTHDR_ICMP:
            break;
        case NEXTHDR_FRAGMENT:
            return &print_ipv6_frag;
        default:
            return NULL;
            //printf("Next header unknow or end-state : %d\n", next_header);
    }
    return NULL;
}

void print_ipv6(void *pkt) {
    struct parser_ipv6 *ip6hdr = (struct parser_ipv6 *) pkt;
    if(ip6hdr == NULL) {
        fprintf(stderr, "Header IPv6 is NULL\n");
        return;
    }

    char str_src[INET6_ADDRSTRLEN];
    char str_dst[INET6_ADDRSTRLEN];

    if( ! inet_ntop(AF_INET6, &(ip6hdr->src), str_src, INET6_ADDRSTRLEN) ) {
        fprintf(stderr, "inet_ntop error with source IPv6 parsing\n");
        return ;
    }

    if( ! inet_ntop(AF_INET6, &(ip6hdr->dst), str_dst, INET6_ADDRSTRLEN)) {
        fprintf(stderr, "inet_ntop error with destination IPv6 parsing\n");
        return ;
    }

    printf("source: %s\n", str_src);
    printf("destination: %s\n", str_dst);
    printf("Next header: %u\n", ip6hdr->next_header);
    printf("length: %u\n", ntohs(ip6hdr->length));
    printf("HOP limit: %u\n", ip6hdr->hop_limit);
    printf("Version: %u\n", ntohs(ip6hdr->version_class_flow) >> 12);
    printf("Class: %u\n", (ntohs(ip6hdr->version_class_flow) >> 4) & 0xff);
    printf("Start flow: %u\n", ((ntohs(ip6hdr->version_class_flow) << 16 ) | (ntohs(ip6hdr->version_class_flow) >> 16)) & 0xfffff );
}

void print_ipv6_sr(void *pkt) {
    int i;
    struct parser_ipv6_sr *ip6_sr_hdr = (struct parser_ipv6_sr *) pkt;
    if(ip6_sr_hdr == NULL) {
        fprintf(stderr, "Header IPv6 SR is NULL\n");
        return;
    }

    printf("next header: %u\n", ip6_sr_hdr->next_header);
    printf("hdr length: %u\n", ip6_sr_hdr->hdr_length);
    printf("routing type: %u\n", ip6_sr_hdr->routing_type);
    printf("hmac key: %u\n", ip6_sr_hdr->hmac_key);
    printf("segments left: %u\n", ip6_sr_hdr->seg_left);
    printf("first seg: %u\n", ip6_sr_hdr->first_seg);
    printf("flags: %u\n", ip6_sr_hdr->flags);

    for(i=ip6_sr_hdr->seg_left+1; i>=0; i--) {
        char str_sr[INET6_ADDRSTRLEN];

        if( ! inet_ntop(AF_INET6, ((char *)ip6_sr_hdr)+8+(i*16), str_sr, INET6_ADDRSTRLEN) ) {
            fprintf(stderr, "inet_ntop error with source IPv6 parsing\n");
            return ;
        }
        printf("Seg %d, addr: %s\n", i, str_sr);

    }

}

void print_ipv6_frag(void *pkt){
    struct parser_ipv6_frag *ip6_frag_hdr = (struct parser_ipv6_frag *) pkt;
    if(ip6_frag_hdr == NULL) {
        fprintf(stderr, "Header IPv6 fragmentation is NULL\n");
        return;
    }

    printf("next header: %u\n", ip6_frag_hdr->next_header);
    printf("reserved 1: %u\n", ip6_frag_hdr->reserved_1);
    printf("fragmentation offset: %u\n", (ip6_frag_hdr->start_frag_offset << 5) | (ip6_frag_hdr->end_frag_offset_res_m >> 3));
    printf("reserved 2: %u\n", ip6_frag_hdr->end_frag_offset_res_m & !248);
    printf("M flag: %u\n", ip6_frag_hdr->end_frag_offset_res_m & 1);
    printf("identification: %u\n", ntohl(ip6_frag_hdr->identification));
}


int get_tcp_flag(struct parser_tcp *tcp_hdr, int flag) {
    return ntohs(tcp_hdr->len_res_control) & flag;
}


void print_tcp(void *pkt) {
    struct parser_tcp *tcp_hdr = (struct parser_tcp *) pkt;
    if(tcp_hdr == NULL) {
        fprintf(stderr, "Header TCP is NULL\n");
        return;
    }

    printf("source port: %u\n", ntohs(tcp_hdr->src));
    printf("dest port: %u\n", ntohs(tcp_hdr->dst));
    printf("seq: %u\n", ntohl(tcp_hdr->seq_nbr));
    printf("ack: %u\n", ntohl(tcp_hdr->ack_nbr));
    printf("len_res_control: %u\n", ntohs(tcp_hdr->len_res_control));
    printf("offset: %u\n", (ntohs(tcp_hdr->len_res_control) >> 12) & 0xf);
    printf("res: %u\n", (ntohs(tcp_hdr->len_res_control) >> 9) & 0x7);

    printf("ns: %u\n",  get_tcp_flag(tcp_hdr, FLAG_NS));
    printf("CWR: %u\n", get_tcp_flag(tcp_hdr, FLAG_CWR));
    printf("ECE: %u\n", get_tcp_flag(tcp_hdr, FLAG_ECE));
    printf("URG: %u\n", get_tcp_flag(tcp_hdr, FLAG_URG));
    printf("ACK: %u\n", get_tcp_flag(tcp_hdr, FLAG_ACK));
    printf("PSH: %u\n", get_tcp_flag(tcp_hdr, FLAG_PSH));
    printf("RST: %u\n", get_tcp_flag(tcp_hdr, FLAG_RST));
    printf("SYN: %u\n", get_tcp_flag(tcp_hdr, FLAG_SYN));
    printf("FIN: %u\n", get_tcp_flag(tcp_hdr, FLAG_FIN));

    printf("window: %u\n", ntohs(tcp_hdr->win));
    printf("URG pointer: %u\n", ntohs(tcp_hdr->urgent));

}




/*
 * TODO: delete old library
 */


void *parse_ipv6(char *data, int *offset, int *next_header){
    struct parser_ipv6 *ip6hdr = (struct parser_ipv6 *) (data + *offset);


    char str_src[INET6_ADDRSTRLEN];
    char str_dst[INET6_ADDRSTRLEN];

    if( ! inet_ntop(AF_INET6, &(ip6hdr->src), str_src, INET6_ADDRSTRLEN) ) {
        fprintf(stderr, "inet_ntop error with source IPv6 parsing\n");
        return NULL;
    }

    if( ! inet_ntop(AF_INET6, &(ip6hdr->dst), str_dst, INET6_ADDRSTRLEN)) {
        fprintf(stderr, "inet_ntop error with destination IPv6 parsing\n");
        return NULL;
    }

    if(offset != NULL)
        *offset += 40;
    if(next_header != NULL)
        *next_header = ip6hdr->next_header;

    return ip6hdr;
}

void *parse_ipv6_sr(char *data, int *offset, int *next_header) {
    struct parser_ipv6_sr *ip6_sr_hdr = (struct parser_ipv6_sr *) (data + *offset);

    // hdr_length is on 8-bytes unit without the first 8 bytes header
    if(offset != NULL)
        *offset += 8 + 8 * ip6_sr_hdr->hdr_length;
    if(next_header != NULL)
        *next_header = ip6_sr_hdr->next_header;

    return ip6_sr_hdr;
}

void *parse_tcp(char *data, int *offset, int *next_header) {
    struct parser_tcp *tcp_hdr = (struct parser_tcp *) (data + *offset);

    // data_offset is set on 4-bytes unit from start header
    *offset += 4 * ((ntohs(tcp_hdr->len_res_control) >> 12) & 0xf);
    *next_header = -1;

    return tcp_hdr;
}
