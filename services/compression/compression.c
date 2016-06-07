#include "compression.h"

//int SET_TASK1;
//int SET_TASK2;

extern int verbose_flag;

int mtu_buffer = 9000;
int compression_level = 1;
int compression_type = 0;


int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused)
{
    return NL_SKIP;
}

void usage(char *av0)
{
    fprintf(stderr, "Usage: %s --MODE [-cdv] [--type TYPE] [--memory] ip_segment\n\tMODE := \tcompress|decompress\n\tTYPE := \tlz0|lz4|slz\n\t--memory -m \t memory in Mb available for the socket (default=128)\n", av0);
    exit(-1);
}

int main(int argc, char **argv)
{
    struct seg6_sock *sk;
    struct in6_addr in6;
    struct nlmsghdr *msg;
    struct nlmem_cb cb;
    int ret, init;

    int c, i, nb_threads = 0;
    static int service_mode = -1; // COMPRESS or DECOMPRESS
    int memory = 128;

    // args parameters
    char *ip6_binding = malloc(INET6_ADDRSTRLEN * sizeof(char));


    // Get opt http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    while (1)
    {
        static struct option long_options[] =
                {
                        {"verbose", no_argument, NULL, 'v'},
                        {"compress", no_argument, &service_mode, COMPRESS},
                        {"decompress", no_argument, &service_mode, DECOMPRESS},
                        {"level", required_argument, NULL, 'l'},
                        {"type", required_argument, NULL, 't'},
                        {"memory", required_argument, NULL, 'm'},
                        {"buffer", required_argument, NULL, 'b'},
                        {"threads", required_argument, NULL, 'n'},
                        {"test", required_argument, NULL, 'p'},
                        {"test2", required_argument, NULL, 'q'},
                        {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "vcdl:t:m:b:n:", //p:q:",
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


            case 'q':
                SET_TASK1 = atoi(optarg);
                break;

            case 'p':
                SET_TASK2 = atoi(optarg);
                break;

            case 'v':
                verbose_flag = 1;
                break;

            case 'c':
                service_mode = COMPRESS;
                break;

            case 'd':
                service_mode = DECOMPRESS;
                break;

            case 'm':
                memory = atoi(optarg);
                break;

            case 'b':
                mtu_buffer = atoi(optarg);
                break;

            case 'l':
                compression_level = atoi(optarg);
                break;

            case 'n':
                nb_threads = atoi(optarg);
                break;

            case 't':

                for(i=0; i <= COMPRESSION_TYPE_MAX; i++){
                    if(!strncmp(optarg, COMPRESSION_TYPE_ARG[i], 3)) {
                        compression_type = i;
                    }
                }
                break;

            default:
                usage(argv[0]);
        }
    }


    printf("Memory allocated for the socket : %d\n", memory);
    printf("Compression type: %s\n", COMPRESSION_TYPE_ARG[compression_type]);
    printf("Mode (1=compress, 0=decompress): %d\n", service_mode);
    printf("MTU used : %d\n", mtu_buffer);
    printf("Threads : %d\n", nb_threads);


    if (verbose_flag){
        puts("verbose flag is set\n");
    }

    if (optind >= argc || service_mode == -1){
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


    init = (*COMPRESSION_TYPE_FCT_INIT[compression_type])();
    if (init) {
        fprintf(stderr, "initialization compression: %s\n", strerror(-init));
        return init;
    }

    ret = __synchrone_service_launch(sk, &in6,
            COMPRESSION_TYPE_FCT_NLMEM[compression_type][service_mode],
            COMPRESSION_TYPE_FCT_SEG6[compression_type][service_mode],
            nb_threads, THREADING_MUTEX);

    if (ret)
        fprintf(stderr, "seg6_send_and_recv(): %s\n", strerror(-ret));

    seg6_socket_destroy(sk);
    free(ip6_binding);

    return 0;
}
