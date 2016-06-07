

#include <stdlib.h>
#include <stdint.h>
#include "parsing.h"
#include "uthash.h"


#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include <pthread.h>

#include "bulk_buffer.h"

#define MD5_DIGEST_LENGTH 16

#ifndef FLOW_H
#define FLOW_H

extern pthread_mutex_t lock;
extern pthread_rwlock_t rwlock;
extern size_t BUFFER_SIZE;

struct tcp_flow_list {
    struct bulk_list *b;
    char ip_port[TCP_HASH_SIZE];
    UT_hash_handle hh;
};

struct tcp_flow_list_2 {
    struct bulk_list *b;
    char * key;
    struct tcp_flow_list_2 *sub;
    UT_hash_handle hh;
};



struct tcp_flow_2 {
    struct bulk_buffer *b;
    unsigned int seq;
    // FIXME
    unsigned int opt;

    char * key;
    struct tcp_flow_2 *sub;
    UT_hash_handle hh;
};


struct bulk_flow {
    struct bulk_buffer *b;
    int id;
    UT_hash_handle hh;
};


/*           _    _
*     _ __ | | _| |_
*    | '_ \| |/ / __|
*    | |_) |   <| |_
*    | .__/|_|\_\\__|
*    |_|
*/

struct bulk_flow* get_bulk_flow(struct bulk_flow **flows, int flow_id);
struct bulk_flow* add_bulk_flow(struct bulk_flow** flows, int flow_id);
struct bulk_flow* get_bulk_flow_or_create(struct bulk_flow **flows, int flow_id);

int init_bulk_flow(struct bulk_flow **b, size_t size, int flow_id) ;

/*
*    | |_ ___ _ __
*    | __/ __| '_ \
*    | || (__| |_) |
*     \__\___| .__/
*            |_|
*/


struct tcp_flow_2* get_tcp_flow_2_or_create(struct tcp_flow_2 **flows,
        char *hash1, char *hash2);




/*     _ _     _
*    | (_)___| |_
*    | | / __| __|
*    | | \__ \ |_
*    |_|_|___/\__|
*/


struct tcp_flow_list_2* get_tcp_flow_list_2_or_create(struct tcp_flow_list_2 **flows,
        char *hash1, char *hash2);
struct tcp_flow_list_2 *add_tcp_flow_list_2(struct tcp_flow_list_2 **flows, char *hash1, char *hash2);
struct tcp_flow_list_2* get_tcp_flow_list_2(struct tcp_flow_list_2 **flows, char *hash1, char *hash2);
struct tcp_flow_list_2 *add_tcp_flow_list_2_bis(struct tcp_flow_list_2 **flow,
        struct tcp_flow_list_2 **flows, char *hash, int size, int param);



#endif
