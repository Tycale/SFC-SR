#include "service.h"

extern int stat_else;
extern int stat_valid_0;
extern int stat_valid;
extern int stat_skip;
extern int stat_copy;
extern int stat_copy_0;


int SET_TASK1 = -1;
int SET_TASK2 = -1;

int THREADING_MODE = THREADING_LFQ;
char * THREADING_ARG[] = { "lfq", "mutex"};



extern struct in_packet_buffer *in_buffer;
extern struct lfqueue *nfq;



void forward_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    int pkt_len;
    char *pkt_data;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    send_ip6_packet(sk, pkt_len, pkt_data);
}


int synchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread) {

    return __synchrone_service_launch(sk, in6, nlmem_callback, seg6_callback, n_thread,
            THREADING_MODE);
}

int asynchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread){

    return __asynchrone_service_launch(sk, in6, nlmem_callback, seg6_callback, n_thread,
            THREADING_MODE);
}

int __synchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread, int t_type) {

    return service_launch(sk, in6, nlmem_callback, seg6_callback, n_thread, t_type, 0);
}

int __asynchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread, int t_type){

    return service_launch(sk, in6, nlmem_callback, seg6_callback, n_thread, t_type, 0x1);
}


int service_launch(struct seg6_sock *sk, struct in6_addr *in6, nlmem_cb_t nlmem_callback,
        service_t seg6_callback, int n_thread, int t_type, int type) {

    stat_valid_0=0;
    stat_valid=0;
    stat_skip=0;
    stat_else=0;
    stat_copy=0;
    stat_copy_0=0;
    signal(SIGINT, sigint);

    if( n_thread == 0 )
        return seg6_service_launch(sk, in6, nlmem_callback, seg6_callback, type);
    else
        return threaded_service_launch(sk, in6, nlmem_callback, seg6_callback,
                n_thread, type, t_type);
}


int threaded_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback,
        int n_thread, int service_type, int thr_mode) {

    int i, res;
    pthread_t * threads;

    init_threading(sk, &threads, n_thread, thr_mode);

    for(i=0; i<n_thread; i++)
        launch_consumer(threads+i, seg6_callback, thr_mode);

    if(SET_TASK1 >= 0)
        cpuset(SET_TASK1);

    if(thr_mode == THREADING_MUTEX)
        res = seg6_service_launch(sk, in6, nlmem_callback, &producer_mut_service, service_type);
    else
        res = seg6_service_launch(sk, in6, nlmem_callback, &producer_lfq_service, service_type);

    for(i=0; i<n_thread; i++)
        pthread_join(threads[i], NULL);

    free_threading(thr_mode);
    free(threads);

    return res;
}


int seg6_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
                   nlmem_cb_t nlmem_callback, service_t seg6_callback, int type ) {

    struct nlmsghdr *msg;
    struct nlmem_cb cb;

    seg6_set_callback(sk, SEG6_CMD_PACKET_IN, seg6_callback);
    msg = nlmem_msg_create(sk->nlm_sk, SEG6_CMD_ADDBIND, NLM_F_REQUEST);

    nlmem_nla_put(sk->nlm_sk, msg, SEG6_ATTR_DST, sizeof(struct in6_addr), in6);
    nlmem_nla_put_u8(sk->nlm_sk, msg, SEG6_ATTR_BIND_OP, SEG6_BIND_SERVICE);
    nlmem_nla_put_u32(sk->nlm_sk, msg, SEG6_ATTR_FLAGS, type);
    nlmem_nla_put(sk->nlm_sk, msg, SEG6_ATTR_BIND_DATA, 0, NULL);
    nlmem_nla_put_u32(sk->nlm_sk, msg, SEG6_ATTR_BIND_DATALEN, 0);

    memset(&cb, 0, sizeof(cb));

    /*
     * user-defined callback for ack: just skip to next packet.
     * default action is to stop processing upon ack reception but
     * we do not want that with binding segment processing
     */
    nlmem_set_cb(&cb, NLMEM_CB_ACK, nlmem_callback, NULL);

    set_thr_id(0);

    return seg6_send_and_recv(sk, msg, &cb);
}


void sigint(int sig __unused) {

    printf(" valid  : %d\n valid0 : %d\n skip   : %d\n else   : %d\n copy   : %d\n copy0  : %d\n  ", stat_valid, stat_valid_0, stat_skip, stat_else, stat_copy, stat_copy_0);

    exit(0);
}
