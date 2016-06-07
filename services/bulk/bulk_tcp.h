#ifndef BULK_TCP_H
#define BULK_TCP_H

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



void bulk_tcp(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh);

void bulk_tcp_list(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh);


#endif
