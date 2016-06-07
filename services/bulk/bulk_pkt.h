#ifndef BULK_PKT_H
#define BULK_PKT_H

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
#include "../lib/utils.h"


/*
 *
 */
void send_debulk_buffer(struct seg6_sock *sk, struct parser_ipv6_sr *bulk_sr,
        int debulk_len, char *debulk_data, int bulk_hdr_size);

/*
 *
 */
void debulk_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh);

/*
 *
 */
void bulk_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh);


#endif
