#ifndef SR_COMPRESSION_LZ4_H
#define SR_COMPRESSION_LZ4_H


#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"
#include "../lib/service.h"

#include "lz4.h"

#define FAKE_LZ4 1

void compress_lz4_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

void decompress_lz4_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

int init_lz4(void);

#endif // SR_COMPRESSION_LZ4_H

