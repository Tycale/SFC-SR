#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"

#include <linux/if.h>
#include <linux/if_tun.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <stdint.h>
#include <string.h>

#include <getopt.h>


#ifndef SR6_SERVICE_CHAINING_TUN_INTERFACE_H
#define SR6_SERVICE_CHAINING_TUN_INTERFACE_H

int tun_alloc(char *dev, int flags);

void tun_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *nlh);

void usage(char *av0);

int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused);

int main(int argc, char **argv);

#endif //SR6_SERVICE_CHAINING_TUN_INTERFACE_H
