
#include "multi_threading.h"

extern int SET_TASK1;
extern int SET_TASK2;

// General threading
extern int THREADING_MODE;

struct lfqueue *nfq;
struct in_packet_buffer *in_buffer;

__thread size_t __thr_id;

void producer_mut_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr* nlh) {
    struct wrapper *el;
    struct in_packet_buffer *buf;
    int pkt_len;
    char *pkt_data;

    buf = in_buffer;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    producer_pc_wait(buf);

    el = buf->buffer + buf->writepos;
    el->attrs = attrs;
    el->nlh = nlh;
    advance_wr_pos(buf);

    producer_pc_post(buf);
}

void producer_lfq_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr* nlh) {
    struct wrapper el;
    int pkt_len;
    char *pkt_data;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    el.attrs = attrs;
    el.nlh = nlh;

    lfq_push(nfq, &el);
}

void *consumer_mut_service(void * args) {
    struct in_packet_buffer *buf;
    struct wrapper *el;
    struct seg6_sock *sk, *sk_in;

    struct nlattr **attrs;
    struct nlmsghdr* nlh;
    struct nl_mmap_hdr *hdr;

    int pkt_len;
    char *pkt_data;

    sk = seg6_socket_create(MEM_SOCK_OUT*getpagesize(), 64);

    service_t *service = (service_t *) args;

    buf = in_buffer;

    if(SET_TASK2 >= 0)
        cpuset(SET_TASK2);

    for(;;) {
        consumer_pc_wait(buf);

        el = buf->buffer + buf->readpos;
        attrs = el->attrs;
        nlh = el->nlh;
        advance_rd_pos(buf);

        consumer_pc_post(buf);

        pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
        pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

        service(sk, attrs, nlh);

        hdr = ((void *)nlh - NL_MMAP_HDRLEN);
        hdr->nm_status = NL_MMAP_STATUS_UNUSED;

        free(attrs);
    }
}



void *consumer_lfq_service(void * args) {
    struct wrapper el;

    struct seg6_sock *sk;
    struct nlattr **attrs;
    struct nlmsghdr* nlh;
    struct nl_mmap_hdr *hdr;

    int pkt_len;
    char *pkt_data;
    struct thr_arg *t_arg = args;

    sk = seg6_socket_create(MEM_SOCK_OUT*getpagesize(), 64);

    service_t *service = (service_t *) t_arg->callback;

    set_thr_id(t_arg->tid);
    free(t_arg);

    if(SET_TASK2 >= 0)
        cpuset(SET_TASK2);

    for(;;) {
        lfq_pop(nfq, &el);

        attrs = el.attrs;
        nlh = el.nlh;

        pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
        pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

        service(sk, attrs, nlh);

        hdr = ((void *)nlh - NL_MMAP_HDRLEN);
        hdr->nm_status = NL_MMAP_STATUS_UNUSED;

        free(attrs);
    }
}


void init_threading(struct seg6_sock *sk, pthread_t **threads, int n_thread, int thr_mode) {
    *threads = (pthread_t *) malloc(n_thread*sizeof (pthread_t));

    sk->nlm_sk->delayed_release = 1;

    if(thr_mode == THREADING_MUTEX)
        init_pc_buffer(&in_buffer);
    else
        nfq = lfq_init(1, n_thread, sizeof(struct wrapper));


}

void launch_consumer(pthread_t *thread, service_t seg6_callback, int thr_mode) {
    struct thr_arg *t_arg;
    static int id=0;

    if(thr_mode == THREADING_MUTEX)
        pthread_create(thread, NULL, &consumer_mut_service, (void *) seg6_callback);
    else {
        t_arg = malloc(sizeof(*t_arg));
        t_arg->callback = (void *) seg6_callback;
        t_arg->tid = id++;
        pthread_create(thread, NULL, &consumer_lfq_service, (void *) t_arg);
    }
}

void free_threading(int thr_mode) {
    if(thr_mode == THREADING_MUTEX)
        free_pc_buffer(in_buffer);
    else
        lfq_destroy(nfq);
}
