
#include "flow.h"


extern int verbose_flag;

pthread_mutex_t lock;

pthread_rwlock_t rwlock;

//           _    _
//     _ __ | | _| |_
//    | '_ \| |/ / __|
//    | |_) |   <| |_
//    | .__/|_|\_\\__|
//    |_|

int init_bulk_flow(struct bulk_flow **b, size_t size, int flow_id) {
    if((*b = (struct bulk_flow *) malloc(sizeof(struct bulk_flow))) == NULL) {
        fprintf(stderr, "bulk_flow cannot be malloc\n");
        return -1;
    }

    if(init_bulk_buffer(&(*b)->b, size) != 0) {
        free(*b);
        return -1;
    }

    (*b)->id = flow_id;

    return 0;
}


struct bulk_flow* get_bulk_flow(struct bulk_flow **flows, int flow_id) {
    struct bulk_flow *flow;
    HASH_FIND_INT( *flows, &flow_id, flow );
    return flow;
}

struct bulk_flow *add_bulk_flow(struct bulk_flow **flows, int flow_id) {
    struct bulk_flow *flow;
    init_bulk_flow(&flow, BUFFER_SIZE, flow_id);
    HASH_ADD_INT( *flows, id, flow );
    return flow;
}

struct bulk_flow* get_bulk_flow_or_create(struct bulk_flow **flows, int flow_id) {
    struct bulk_flow *res;

    pthread_rwlock_rdlock(&rwlock);
    if((res = get_bulk_flow(flows, flow_id)) == NULL) {
        pthread_rwlock_unlock(&rwlock);

        pthread_rwlock_wrlock(&rwlock);
        if((res = get_bulk_flow(flows, flow_id)) == NULL)
            res = add_bulk_flow(flows, flow_id);
    }

    pthread_rwlock_unlock(&rwlock);
    return res;
}


//     _
//    | |_ ___ _ __
//    | __/ __| '_ \
//    | || (__| |_) |
//     \__\___| .__/
//            |_|

struct tcp_flow_2* get_tcp_flow_2bis(struct tcp_flow_2 **flows, char *hash, unsigned int size) {
    struct tcp_flow_2 *flow;
    HASH_FIND(hh, *flows, hash, size, flow );
    return flow;
}

struct tcp_flow_2 *add_tcp_flow_2bis(struct tcp_flow_2 **flows, char *hash, unsigned int size) {
    struct tcp_flow_2 *flow;

    char * tmp = malloc(size);
    memcpy(tmp, hash, size);
    init_tcp_flow_2(&flow, BUFFER_SIZE, tmp, 0);
    HASH_ADD_KEYPTR(hh, *flows, flow->key, size, flow );

    return flow;
}

struct tcp_flow_2* get_tcp_flow_2_or_create(struct tcp_flow_2 **flows,
        char *hash1, char *hash2) {
    struct tcp_flow_2 *res, *ressub;

    pthread_rwlock_rdlock(&rwlock);
    if((res = get_tcp_flow_2bis(flows, hash1, 36)) == NULL) {
        pthread_rwlock_unlock(&rwlock);

        pthread_rwlock_wrlock(&rwlock);
        if((res = get_tcp_flow_2bis(flows, hash1, 36)) == NULL)
            res = add_tcp_flow_2bis(flows, hash1, 36);
    }
    pthread_rwlock_unlock(&rwlock);

    pthread_rwlock_rdlock(&rwlock);
    if((ressub = get_tcp_flow_2bis(&(res->sub), hash2, 4)) == NULL){
        pthread_rwlock_unlock(&rwlock);

        pthread_rwlock_wrlock(&rwlock);
        if((ressub = get_tcp_flow_2bis(&(res->sub), hash2, 4)) == NULL)
            ressub = add_tcp_flow_2bis(&(res->sub), hash2, 4);
    }
    pthread_rwlock_unlock(&rwlock);


    return ressub;
}



int init_tcp_flow_2(struct tcp_flow_2 **b, size_t size, char *hash, int bulk) {
    if((*b = (struct tcp_flow_2 *) malloc(sizeof(struct tcp_flow_2))) == NULL) {
        fprintf(stderr, "tcp_flow_2 cannot be malloc\n");
        return -1;
    }

    if(init_bulk_buffer(&(*b)->b, size) != 0) {
        free(*b);
        return -1;
    }

    (*b)->sub = NULL;
    (*b)->key = hash;

    return 0;
}


//     _               _ _     _
//    | |_ ___ _ __   | (_)___| |_
//    | __/ __| '_ \  | | / __| __|
//    | || (__| |_) | | | \__ \ |_
//     \__\___| .__/  |_|_|___/\__|
//            |_|

struct tcp_flow_list* get_tcp_flow_list(struct tcp_flow_list **flows, char *hash) {
    struct tcp_flow_list *flow;
    HASH_FIND_STR( *flows, hash, flow );
    return flow;
}

struct tcp_flow_list *add_tcp_flow_list(struct tcp_flow_list **flows, char *hash) {
    struct tcp_flow_list *flow;
    init_tcp_flow_list(&flow, BUFFER_SIZE, hash);
    HASH_ADD_STR( *flows, ip_port, flow );
    return flow;
}

struct tcp_flow_list* get_tcp_flow_list_or_create(struct tcp_flow_list **flows, char *hash) {
    struct tcp_flow_list *res;
    if((res = get_tcp_flow_list(flows, hash)) == NULL) {
        pthread_mutex_lock(&lock);
        if((res = get_tcp_flow_list(flows, hash)) == NULL)
            res = add_tcp_flow_list(flows, hash);
        pthread_mutex_unlock(&lock);
    }

    return res;
}



int init_tcp_flow_list(struct tcp_flow_list **b, size_t size, char *hash) {
    if((*b = (struct tcp_flow_list *) malloc(sizeof(struct tcp_flow_list))) == NULL) {
        fprintf(stderr, "tcp_flow_list cannot be malloc\n");
        return -1;
    }

    if(init_bulk_list(&(*b)->b) != 0) {
        free(*b);
        return -1;
    }

    memcpy((*b)->ip_port, hash, TCP_HASH_SIZE);

    return 0;
}




//     _               _ _     _
//    | |_ ___ _ __   | (_)___| |_
//    | __/ __| '_ \  | | / __| __|
//    | || (__| |_) | | | \__ \ |_
//     \__\___| .__/  |_|_|___/\__|
//            |_|


struct tcp_flow_list_2* get_tcp_flow_list_2(struct tcp_flow_list_2 **flows, char *hash1, char *hash2) {
    struct tcp_flow_list_2 *flow, *sub;
    HASH_FIND(hh, *flows, hash1, (unsigned) 32, flow );
    if(flow == NULL)
        return NULL;

    HASH_FIND(hh, flow->sub, hash2, (unsigned) 4, sub );
    if(sub == NULL)
        add_tcp_flow_list_2_bis(&sub, &(flow->sub), hash2, 4, 1 );

    return sub;
}

struct tcp_flow_list_2 *add_tcp_flow_list_2(struct tcp_flow_list_2 **flows, char *hash1, char *hash2) {
    struct tcp_flow_list_2 *flow, *sub;
//    init_tcp_flow_list_2(&flow, BUFFER_SIZE, hash1, 0);
//    HASH_ADD_KEYPTR(hh, *flows, flow->key, 32, flow );

    add_tcp_flow_list_2_bis(&flow, flows, hash1, 32, 0 );
    flow->sub = NULL;
    add_tcp_flow_list_2_bis(&sub, &(flow->sub), hash2, 4, 1 );

//    init_tcp_flow_list_2(&sub, BUFFER_SIZE, hash2, 1);
//    HASH_ADD_KEYPTR(hh, flow->sub, sub->key, 4, sub );

    return sub;
}

struct tcp_flow_list_2 *add_tcp_flow_list_2_bis(struct tcp_flow_list_2 **flow,
        struct tcp_flow_list_2 **flows, char *hash, int size, int param) {

    init_tcp_flow_list_2(flow, BUFFER_SIZE, hash, param);
    HASH_ADD_KEYPTR(hh, *flows, (*flow)->key, size, *flow );
}

struct tcp_flow_list_2* get_tcp_flow_list_2_or_create(struct tcp_flow_list_2 **flows,
        char *hash1, char *hash2) {
    struct tcp_flow_list_2 *res;
    if((res = get_tcp_flow_list_2(flows, hash1, hash2)) == NULL) {
        pthread_mutex_lock(&lock);
        if((res = get_tcp_flow_list_2(flows, hash1, hash2)) == NULL)
            res = add_tcp_flow_list_2(flows, hash1, hash2);
        pthread_mutex_unlock(&lock);
    }

    return res;
}



int init_tcp_flow_list_2(struct tcp_flow_list_2 **b, size_t size, char *hash, int bulk) {
    if((*b = (struct tcp_flow_list_2 *) malloc(sizeof(struct tcp_flow_list_2))) == NULL) {
        fprintf(stderr, "tcp_flow_list_2 cannot be malloc\n");
        return -1;
    }

    if(bulk){
        if(init_bulk_list(&(*b)->b) != 0) {
            free(*b);
            return -1;
        }
    }

    (*b)->sub = NULL;
    (*b)->key = hash;

    return 0;
}
