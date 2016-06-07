
#include "utils.h"
#include <pthread.h>

pthread_mutex_t sending;

int verbose_flag = 0;

void verbose(char *str) {
    if(verbose_flag)
        printf("%s", str);
}

void cpuset(int c) {

    int s, j; cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(c, &cpuset);

    s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        fprintf(stderr, "pthread_setaffinity_np");
        exit(0);
    }
}


double timevaldiff_double(struct timeval *starttime, struct timeval *finishtime) {
    double msec;
    msec=(double) (finishtime->tv_sec-starttime->tv_sec)*1000;
    msec+= (double) (finishtime->tv_usec-starttime->tv_usec)/1000;
    return msec;
}

long timevaldiff(struct timeval *starttime, struct timeval *finishtime) {
    long msec;
    msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
    msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
    return msec;
}

void send_ip6_packet(struct seg6_sock *sk, int len, char* data){
    struct nlmsghdr *msg;
    struct nl_mmap_hdr *hdr;


    pthread_mutex_lock(sk->nlm_sk->lock);

    msg = nlmem_msg_create(sk->nlm_sk, SEG6_CMD_PACKET_OUT, NLM_F_REQUEST);

    if(msg == NULL) {
        hdr = current_tx_frame(sk->nlm_sk);
        printf("OH MY GOD = %p\n", hdr->nm_status);
        return;
    }

    nlmem_nla_put_u32(sk->nlm_sk, msg, SEG6_ATTR_PACKET_LEN, len);
    nlmem_nla_put(sk->nlm_sk, msg, SEG6_ATTR_PACKET_DATA, len, data);
    nlmem_send_msg(sk->nlm_sk, msg);

    pthread_mutex_unlock(sk->nlm_sk->lock);
}

void hexdump(void *mem, unsigned int len) {
    unsigned int i, j;

    for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
        if(i % HEXDUMP_COLS == 0) /* print offset */
            printf("0x%06x: ", i);

        if(i < len) /* print hex data */
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        else        /* end of block, just aligning for ASCII dump */
            printf("   ");

        /* print ASCII dump */
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= len)                      /* end of block, not really printing */
                    putchar(' ');
                else if(isprint(((char*)mem)[j])) /* printable char */
                    putchar(0xFF & ((char*)mem)[j]);
                else                              /* other char */
                    putchar('.');
            }
            putchar('\n');
        }
    }
}
