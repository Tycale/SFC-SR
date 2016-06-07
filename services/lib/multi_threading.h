#ifndef MULTI_H
#define MULTI_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>


#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <asm/types.h>
#include <linux/types.h>
#include <signal.h>


#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include "pc_buffer.h"
#include "lfq.h"

#include "service.h"

#define MEM_SOCK_OUT 128

extern struct lfqueue *nfq;
extern struct in_packet_buffer *in_buffer;

struct thr_arg {
    void *callback;
    int tid;
};

typedef void (service_t)(struct seg6_sock *, struct nlattr **, struct nlmsghdr *);

void producer_mut_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr* nlh);
void producer_lfq_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr* nlh);
void *consumer_mut_service(void * args);
void *consumer_lfq_service(void * args);
void init_threading(struct seg6_sock *sk, pthread_t **threads, int n_thread, int thr_mode);
void launch_consumer(pthread_t *thread, service_t seg6_callback, int thr_mode);
void free_threading(int thr_mode);



#endif // MULTI_H
