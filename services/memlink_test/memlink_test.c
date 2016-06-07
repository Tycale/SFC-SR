#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"

#include "../lib/parsing.h"
#include "../lib/service.h"

#include <getopt.h>


extern int verbose_flag;

void memlink_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *nlh __attribute__((unused)))
{
    int pkt_len;
    char *pkt_data;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    //printf("sent\n");

    send_ip6_packet(sk, pkt_len, pkt_data);
}

void usage(char *av0)
{
    fprintf(stderr, "Usage: %s [--threads T] [--memory M] [-v] segment\n", av0);
    exit(-1);
}


int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused)
{
    return NL_SKIP;
}

int main(int argc, char **argv)
{
    struct seg6_sock *sk;
    struct in6_addr in6;
    int ret;

    int memory = 128, nb_threads = 0, c = 0;
    char *ip6_binding = malloc(INET6_ADDRSTRLEN * sizeof(char));

    // Get opt http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    while (1)
    {
        static struct option long_options[] =
                {
                        {"verbose", no_argument, NULL, 'v'},
                        {"memory", required_argument, NULL, 'm'},
                        {"threads", required_argument, NULL, 'n'},
                        {"mode", required_argument, NULL, 'c'},
                        {"test", required_argument, NULL, 'p'},
                        {"test2", required_argument, NULL, 'q'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        int i;

        c = getopt_long (argc, argv, "vm:n:c:p:q:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'v':
                verbose_flag = 1;
                break;

            case 'm':
                memory = atoi(optarg);
                break;

            case 'n':
                nb_threads = atoi(optarg);
                break;

            case 'q':
                SET_TASK1 = atoi(optarg);
                break;

            case 'p':
                SET_TASK2 = atoi(optarg);
                break;

            case 'c':
                for(i=0; i<=THREADING_MAX; i++) {
                    if (!strcmp(optarg, THREADING_ARG[i])) {
                        THREADING_MODE = i;
                    }
                }
                break;

            default:
                usage(argv[0]);
        }
    }


    printf("Memory allocated for the socket : %d\n", memory);
    printf("Threads : %d\n", nb_threads);


    if (verbose_flag){
        puts("verbose flag is set\n");
    }

    if (optind >= argc){
        usage(argv[0]);
    }
    else {
        strcpy(ip6_binding, argv[optind]);
        optind++;
    }

    if (optind < argc)
    {
        usage(argv[0]);
    }

    inet_pton(AF_INET6, ip6_binding, &in6);

    /*
     * seg6_socket_create(block_size, block_nr)
     * mem usage = block_size * block_nr * 2
     * default settings = 8MB usage for 4K pages
     * increase block_size and not block_nr if needed
     */
    sk = seg6_socket_create(memory*getpagesize(), 64);

    ret = __synchrone_service_launch(sk, &in6, nl_recv_ack, memlink_packet_in, nb_threads, THREADING_MODE);

    if (ret)
        fprintf(stderr, "Memlink: seg6_send_and_recv(): %s\n", strerror(ret));

    seg6_socket_destroy(sk);

    return 0;
}
