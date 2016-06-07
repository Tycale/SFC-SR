#ifndef SR_COMPRESSION_H
#define SR_COMPRESSION_H


extern int SET_TASK1;
extern int SET_TASK2;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

#include <getopt.h>


#include "compression_lz4.h"
#include "compression_lzo.h"
#include "compression_gz.h"

#define COMPRESS 1
#define DECOMPRESS 0


int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused);

void usage(char *av0);

int main(int argc, char **argv);

typedef int (*init_cb_t)(void);

enum compression_type {
    LZO_COMPRESSION,
    LZ4_COMPRESSION,
    GZ_COMPRESSION,
    __COMPRESSION_TYPE_MAX
};

#define COMPRESSION_TYPE_MAX (__COMPRESSION_TYPE_MAX - 1)

char * COMPRESSION_TYPE_ARG[] = { "lzo", "lz4", "gz" };

service_t* COMPRESSION_TYPE_FCT_SEG6[COMPRESSION_TYPE_MAX+1][2] = {
        { &decompress_lzo_packet_in, &compress_lzo_packet_in},
        { &decompress_lz4_packet_in, &compress_lz4_packet_in},
        { &decompress_gz_packet_in, &compress_gz_packet_in}
};

nlmem_cb_t COMPRESSION_TYPE_FCT_NLMEM[COMPRESSION_TYPE_MAX+1][2] = {
        { &nl_recv_ack, &nl_recv_ack},
        { &nl_recv_ack, &nl_recv_ack},
        { &nl_recv_ack, &nl_recv_ack}
};

init_cb_t COMPRESSION_TYPE_FCT_INIT[COMPRESSION_TYPE_MAX+1] = { &init_lzo, &init_lz4, &init_gz };

#endif // SR_COMPRESSION_H
