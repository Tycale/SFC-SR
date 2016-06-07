
#include <stdlib.h>
#include <stdint.h>
#include "parsing.h"
#include "uthash.h"


#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include "../lib/lookup3.h"
#include "../lib/GeneralHashFunctions.h"

#define JENKIN_LEN 32

#ifndef BULK_BUFFER_H
#define BULK_BUFFER_H

#define FAKE_HULK 0

extern size_t BUFFER_SIZE;

struct bulk_buffer {
    size_t pos;
    pthread_mutex_t *lock;
    struct timeval last_send;
    char *buffer;
    int nbr;
};


enum op_buffer {
     OP_BUF_UPDLEN=0x1,
     OP_BUF_UPDCKSUM=0x2,
     OP_BUF_UPDALL_LEN=0x4
};

struct tcp_process {
    int tcp_offset;
    int tcp_len;

    int pkt_len;
    char *pkt_data;

    uint32_t opt;
};


void insert_tcp_to_buffer(struct seg6_sock *sk, struct bulk_buffer *buffer,
        struct tcp_process *tcp_info, unsigned int *seq, uint32_t *opt);

void send_buffer(struct seg6_sock *sk, struct bulk_buffer *buffer, int flag);

// Bulk
int init_bulk_buffer(struct bulk_buffer **b, size_t size) ;
int init_buffer(struct bulk_buffer *b, size_t size) ;
void free_bulk_buffer(struct bulk_buffer *b) ;

int available_space(struct bulk_buffer *b, size_t offset);
int add_to_buffer(struct bulk_buffer *b, const char *pkt_data, size_t pkt_len);

#endif
