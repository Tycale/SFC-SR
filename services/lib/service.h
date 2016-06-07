#ifndef SERVICE_H
#define SERVICE_H



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
#include "utils.h"
#include "multi_threading.h"

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif



extern int SET_TASK1;
extern int SET_TASK2;

// General threading
extern int THREADING_MODE;
extern char * THREADING_ARG[];

enum threading_mode {
     THREADING_LFQ,
     THREADING_MUTEX,
     __THREADING_MAX
};
#define THREADING_MAX (__THREADING_MAX - 1)



typedef void (service_t)(struct seg6_sock *, struct nlattr **, struct nlmsghdr *);

void forward_service(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *) ;


int synchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread);

int asynchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread);

int __synchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread, int t_type);

int __asynchrone_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback, int n_thread, int t_type);

int service_launch(struct seg6_sock *sk, struct in6_addr *in6, nlmem_cb_t nlmem_callback,
        service_t seg6_callback, int n_thread, int t_type, int type);

int threaded_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
        nlmem_cb_t nlmem_callback, service_t seg6_callback,
        int n_thread, int service_type, int thr_mode);

int seg6_service_launch(struct seg6_sock *sk, struct in6_addr *in6,
                   nlmem_cb_t nlmem_callback, service_t seg6_callback, int type );



void sigint(int sig __unused);

#endif // SERVICE_H
