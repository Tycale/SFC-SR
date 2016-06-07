

#include <stdlib.h>
#include <stdint.h>
#include "parsing.h"
#include "utlist.h"
#include "bulk_buffer.h"


#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#define MD5_DIGEST_LENGTH 16

#ifndef BULK_LIST_H
#define BULK_LIST_H

extern size_t BUFFER_SIZE;
extern int MIN_CONS_DATA;

extern struct bulk_buffer *glob_buffer;

struct el_bulk_list {
    struct el_bulk_list *next;

    struct tcp_process *tcp;

    uint32_t seq;
    uint32_t len;
    struct timeval last_send;
};

struct bulk_list {
    struct el_bulk_list *list;
    pthread_mutex_t *lock;
};

int init_bulk_list(struct bulk_list **b);
int init_el_bulk_list(struct el_bulk_list **b);
void free_bulk_list(struct bulk_list *b);
void free_list(struct el_bulk_list **list);
void send_list(struct seg6_sock *sk, struct bulk_list *list, int flag);
struct el_bulk_list *insert_tcp_to_list(struct seg6_sock *sk, struct bulk_list *list,
        struct el_bulk_list *el);

#endif
