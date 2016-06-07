#ifndef BULK_H
#define BULK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include <pthread.h>

#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "../lib/parsing.h"
#include "../lib/bulk_buffer.h"
#include "../lib/bulk_list.h"
#include "../lib/flow.h"
#include "../lib/service.h"
#include "../lib/checksum.h"

#include "bulk_pkt.h"
#include "bulk_tcp.h"




extern void *hash_flows2;

typedef void* (timedout_t)(void*);

/*
 *
 */
void* pkt_timedout (void *args);
void* tcp_timedout (void *args);
void* tcp_timedout_2 (void *args);
void* tcp_list_timedout (void *args);
void* tcp_list_2_timedout (void *args);


extern int MIN_CONS_DATA;


struct timedout {
    void **flows;
    struct seg6_sock * sk;
};









int nl_recv_ack(struct nlmem_sock *nlm_sk, struct nlmsghdr *hdr, void *arg);


enum bulk_mode {
    MODE_BULK,
    MODE_DEBULK,
    __BULK_MODE_MAX
};
#define BULK_MODE_MAX (__BULK_MODE_MAX - 1)


enum bulk_type {
    BULK_TYPE_PKT,
    BULK_TYPE_TCP,
    BULK_TYPE_TCP_LIST,
    __BULK_TYPE_MAX
};

#define BULK_TYPE_MAX (__BULK_TYPE_MAX - 1)

char * BULK_TYPE_ARG[] = { "pkt", "tcp", "tcp_list" };

service_t* BULK_TYPE_FCT_SEG6[BULK_MODE_MAX+1][BULK_TYPE_MAX+1] = {
    { &bulk_packet, &bulk_tcp, &bulk_tcp_list },
    { &debulk_packet, &forward_service, &forward_service }
};
nlmem_cb_t BULK_TYPE_FCT_NLMEM[BULK_MODE_MAX+1][BULK_TYPE_MAX+1] = {
    { &nl_recv_ack, &nl_recv_ack, &nl_recv_ack},
    { &nl_recv_ack, &nl_recv_ack, &nl_recv_ack}
};
timedout_t* BULK_TYPE_TIMER[] = { &pkt_timedout, &tcp_timedout_2, &tcp_list_2_timedout };



#endif
