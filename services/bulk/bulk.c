
#include "bulk.h"

#include <openssl/md5.h>


void *hash_flows2 = NULL;

extern int MIN_CONS_DATA;
int reorder = 1;

long DELTA_SEND = 100;

extern size_t BUFFER_SIZE;

extern int verbose_flag;

extern struct bulk_buffer *glob_buffer;



void* tcp_list_timedout (void *args) {
    struct tcp_flow_list **fflows = (struct tcp_flow_list **) ((struct timedout *) args)->flows;
    struct seg6_sock *sk = ((struct timedout *) args)->sk;
    struct tcp_flow_list *f;

    while(1) {
        usleep(DELTA_SEND*500);
        for(f=*fflows; f != NULL; f=f->hh.next)
            send_old_list(sk, f->b, DELTA_SEND, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM);
    }
}

void* tcp_list_2_timedout (void *args) {
    struct tcp_flow_list_2 **fflows = (struct tcp_flow_list_2 **) ((struct timedout *) args)->flows;
    struct seg6_sock *sk = ((struct timedout *) args)->sk;
    struct tcp_flow_list_2 *f, *fl, *tmp1, *tmp2;

    while(1) {
        usleep(DELTA_SEND*1000);
        HASH_ITER(hh, *fflows, fl, tmp1) {
            HASH_ITER(hh, fl->sub, f, tmp2) {
                //printf("Send-old-list!\n");
                send_old_list(sk, f->b, DELTA_SEND, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM);
                //send_list_until(sk, f->b, NULL, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM );

            }
        }
    }
}

void* tcp_timedout_2 (void *args) {
    struct tcp_flow_2 **fflows = (struct tcp_flow_2 **) ((struct timedout *) args)->flows;
    struct seg6_sock *sk = ((struct timedout *) args)->sk;
    struct tcp_flow_2 *f, *fl, *tmp1, *tmp2;

    while(1) {
        usleep(DELTA_SEND*1000);

        HASH_ITER(hh, *fflows, fl, tmp1) {
            HASH_ITER(hh, fl->sub, f, tmp2) {
                send_old_buffer(sk, f->b, DELTA_SEND, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM);
            }
        }
    }
}


void* pkt_timedout (void *args) {
    struct bulk_flow **fflows = (struct bulk_flow **) ((struct timedout *) args)->flows;
    struct seg6_sock *sk = ((struct timedout *) args)->sk;
    struct bulk_flow *f;


    while(1) {
        usleep(DELTA_SEND*1000);
        for(f=*fflows; f != NULL; f=f->hh.next)
            send_old_buffer(sk, f->b, DELTA_SEND, OP_BUF_UPDLEN );
    }
}





int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused) {
    return NL_SKIP;
}

void usage(char *av0)
{
    fprintf(stderr, """Usage: %s ip_segment [-b|--bulk]|[-d|--debulk] [options]\n\
OPTIONS:\n\
    -b, --bulk \n\
        Bulk service mode \n\
    -d, --debulk \n\
        Debulk service mode \n\
    -t, --type      tcp|pkt \n\
        Type for the service \n\
    -r, --reorder   [size] \n\
        Perform some kind of reordering before bulk them \n\
    -B, --buffer    size \n\
        The size in byte to use for the buffer \n\
    -D, --delta     time \n\
        The maximum time in ms to stay in buffer \n\
    -M, --memory    space \n\
        Memory in Mb available for the socket (default=128) \n\
    -v, --verbose \n""", av0);
    exit(-1);
}

int main(int argc, char **argv)
{
    struct seg6_sock *sk;
    struct in6_addr in6;
    int ret, c, memory = 128, service_mode = -1, i, n_threads=0;
    enum bulk_type bulk_type = BULK_TYPE_PKT;

    struct timedout timed_arg;
    pthread_t thread;

    char *ip6_binding = malloc(INET6_ADDRSTRLEN * sizeof(char));

    static struct option long_options[] =
    {
        {"verbose", no_argument      , NULL         , 'v'   },
        {"bulk"   , no_argument      , NULL         , 'b'   },
        {"debulk" , no_argument      , NULL         , 'd'   },
        {"reorder", no_argument      , NULL         , 'r'   },
        {"threads", required_argument, NULL         , 'n'   },
        {"memory" , required_argument, NULL         , 'M'   },
        {"type"   , required_argument, NULL         , 't'   },
        {"buffer" , required_argument, NULL         , 'B'   },
        {"delta"  , required_argument, NULL         , 'D'   },
        {"mincons", required_argument, NULL         , 'c'   },
        {0        , 0                , 0            , 0     }
    };

    // Get opt http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    //while ((c = getopt_long (argc, argv, "vbdm:t:", long_options, &option_index)) != 1) {
    while (1) {
        int option_index = 0;
        c = getopt_long (argc, argv, "vbdm:t:B:n:c:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 0:
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 't':
                for(i=0; i<=BULK_TYPE_MAX; i++) {
                    if (!strcmp(optarg, BULK_TYPE_ARG[i]))
                        bulk_type = i;
                }
                break;
            case 'b':
            case 'd':
                if(service_mode != -1)
                    usage(argv[0]);
                service_mode = (c == 'b') ? MODE_BULK : MODE_DEBULK;
                break;
            case 'c':
                MIN_CONS_DATA = atoi(optarg);
                break;
            case 'n':
                n_threads = atoi(optarg);
                break;
            case 'M':
                memory = atoi(optarg);
                break;
            case 'B':
                BUFFER_SIZE = atoi(optarg);
                break;
            case 'D':
                DELTA_SEND = atoi(optarg);
                break;
            default : usage(argv[0]);
        }
    }

    if (optind >= argc || optind+1 < argc || service_mode == -1) {
        usage(argv[0]);
        exit(0);
    }

    if(MIN_CONS_DATA > (int) BUFFER_SIZE)
        MIN_CONS_DATA = (int) BUFFER_SIZE;

    if (verbose_flag)
        printf("Verbose flag is set\n \
                Memory allocated for the socket : %d\n", memory);

    // Print setup
    printf("Memory allocated for the socket : %d\n", memory);
    printf("Mode (%d=bulk, %d=debulk): %d\n", MODE_BULK, MODE_DEBULK, service_mode);
    printf("Threads : %d\n", n_threads);
    printf("Buffer size : %d\n", BUFFER_SIZE);
    printf("Delta send : %d\n", DELTA_SEND);


    strcpy(ip6_binding, argv[optind]);
    inet_pton(AF_INET6, ip6_binding, &in6);

    /*
     * seg6_socket_create(block_size, block_nr)
     * mem usage = block_size * block_nr * 2
     * default settings = 8MB usage for 4K pages
     * increase block_size and not block_nr if needed
     */

    sk = seg6_socket_create(memory*getpagesize(), 64);

    if (pthread_rwlock_init(&rwlock,NULL) != 0) {
      fprintf(stderr,"lock init failed\n");
      exit(-1);
    }

    if(service_mode == MODE_BULK) {

        init_bulk_buffer(&glob_buffer, BUFFER_SIZE);

        timed_arg.sk = sk;
        timed_arg.flows = &hash_flows2;

        pthread_create(&thread, NULL, BULK_TYPE_TIMER[bulk_type], (void *) &timed_arg);
    }

    ret = synchrone_service_launch(sk, &in6,
            BULK_TYPE_FCT_NLMEM[service_mode][bulk_type],
            BULK_TYPE_FCT_SEG6[service_mode][bulk_type], n_threads);


    if (ret)
        fprintf(stderr, "seg6_send_and_recv(): %s\n", strerror(-ret));

    free_bulk_buffer(glob_buffer);
    seg6_socket_destroy(sk);

    return 0;
}
