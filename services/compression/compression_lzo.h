#ifndef SR_COMPRESSION_LZ0_H
#define SR_COMPRESSION_LZ0_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"

#include <stdint.h>
#include <string.h>

#include <getopt.h>

#include "../../minilzo-2.09/minilzo.h"
#include "../../minilzo-2.09/lzodefs.h"
#include "../../minilzo-2.09/lzoconf.h"

/*************************************************************************
//
 **************************************************************************/

#define COMPRESS 1
#define DECOMPRESS 0

#define FAKE_LZO 1

/* Work-memory needed for compression. Allocate memory in units
 * of 'lzo_align_t' (instead of 'char') to make sure it is properly aligned.
 */
#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

void compress_lzo_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

void decompress_lzo_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused);

int init_lzo(void);

#endif //SR_COMPRESSION_LZ0_H