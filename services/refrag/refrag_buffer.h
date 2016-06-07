
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "../lib/parsing.h"
#include "../lib/service.h"
#include "../lib/checksum.h"
#include "../lib/uthash.h"

#ifndef SR6_SERVICE_CHAINING_REFRAG_BUFFER_H
#define SR6_SERVICE_CHAINING_REFRAG_BUFFER_H

struct bulk_frag {
    char *buffer;
    pthread_mutex_t *lock;
    uint32_t id;
    struct timeval first_received;
    int end_pos;
    int wrote_bytes;
    int next_header;
    UT_hash_handle hh;
};

int init_bulk_frag(struct bulk_frag **b, size_t buffer_size, uint32_t frag_id);

struct bulk_frag *get_bulk_frag(struct bulk_frag **b_frags, uint32_t frag_id);

struct bulk_frag *add_bulk_frag(struct bulk_frag **b_frags, uint32_t flow_id);

struct bulk_frag *get_bulk_frag_or_create(struct bulk_frag **b_frags, uint32_t flow_id);

#endif //SR6_SERVICE_CHAINING_REFRAG_BUFFER_H