#ifndef SR_COMPRESSION_GZ_H
#define SR_COMPRESSION_GZ_H


#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"
#include "../lib/service.h"

#include <zlib.h>

#define FAKE_GZ 1

void compress_gz_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

void decompress_gz_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

int init_gz(void);

#endif // SR_COMPRESSION_GZ_H

