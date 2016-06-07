#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"


extern int verbose_flag;

void count_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs,
        struct nlmsghdr *nlh __unused)
{
    static int cnt;
    static int bytes;
    int pkt_len;
    char *pkt_data;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    verbose("New packet (%u bytes)\n", pkt_len);
    verbose("\tTotal packet = %u\n", ++cnt);
    verbose("\tTotal bytes = %u\n", bytes+=pkt_len);

    /*
    nparse_t *fun;
    show_t *fun_print;

    void *tmp;
    int next_header = 41;
    int offset = 0;
    while((fun = call_parse(next_header)) != NULL){

        // Uncomment to show header
        if(verbose_flag && (fun_print = call_print(next_header)) != NULL )
            fun_print(pkt_data+offset);

        offset += fun(pkt_data, &tmp, offset, &next_header);

        verbose("\n==== new offset : %d \n", offset);
    }
     */
}

void usage(char *av0)
{
    fprintf(stderr, "Usage: %s segment\n", av0);
    exit(-1);
}


int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused)
{
    return NL_SKIP;
}

int main(int ac, char **av)
{
    struct seg6_sock *sk;
    struct in6_addr in6;
    int ret;

    if (ac != 2)
        usage(av[0]);

    inet_pton(AF_INET6, av[1], &in6);

    verbose_flag = 0;

    /*
     * seg6_socket_create(block_size, block_nr)
     * mem usage = block_size * block_nr * 2
     * default settings = 8MB usage for 4K pages
     * increase block_size and not block_nr if needed
     */
    sk = seg6_socket_create(128*getpagesize(), 64);

    ret = asynchrone_service_launch(sk, &in6, &nl_recv_ack, &count_packet_in, 0);
    if (ret)
        fprintf(stderr, "seg6_send_and_recv(): %s\n", strerror(ret));

    seg6_socket_destroy(sk);

    return 0;
}
