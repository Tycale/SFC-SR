
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
#include "../lib/service.h"
#include "../lib/checksum.h"

#include "refrag_buffer.h"

#ifndef SR6_SERVICE_CHAINING_REFRAG_H
#define SR6_SERVICE_CHAINING_REFRAG_H

extern size_t BUFFER_FRAGS_SIZE;

//typedef void* (timeout_t)(void*);

extern pthread_mutex_t hashlist_mutex;

struct thread_timeout_args {
    struct seg6_sock * sk;
};



void* pkt_timeout (void *args);

void add_to_frags(char *pkt_data, int offset, struct bulk_frag *frags, uint16_t frag_offset, size_t frag_size);

void send_frags(struct seg6_sock *sk, int offset, struct bulk_frag *frags);

void refrag_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh);

int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused);


void usage(char *av0);


#endif //SR6_SERVICE_CHAINING_REFRAG_H
