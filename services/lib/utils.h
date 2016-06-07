#ifndef UTILS_H
#define UTILS_H
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>


#ifndef __USE_GNU
#define __USE_GNU
#endif
#define _GNU_SOURCE
#include <sched.h>


#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif

extern int verbose_flag;

void hexdump(void *mem, unsigned int len);

long timevaldiff(struct timeval *starttime, struct timeval *finishtime);

void send_packet(struct seg6_sock *sk, int len, char* data);

#endif
