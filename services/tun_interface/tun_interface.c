#include "tun_interface.h"

#include <unistd.h>

static int verbose_flag = 0;
static char interface_name[IFNAMSIZ] = { 0 };
int fd_tun = -1;

/* From http://backreference.org/2010/03/26/tuntap-interface-tutorial/ */

int tun_alloc(char *dev, int flags) {

    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
     *
     * char *dev: the name of an interface (or '\0'). MUST have enough
     *   space to hold the interface name if '\0' is passed
     * int flags: interface flags (eg, IFF_TUN etc.)
     */

    /* open the clone device */
    if( (fd = open(clonedev, O_RDWR)) < 0 ) {
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

    if (*dev) {
        /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    /* try to create the device */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
        close(fd);
        return err;
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable "dev", so the caller can know
     * it. Note that the caller MUST reserve space in *dev (see calling
     * code below) */
    strcpy(dev, ifr.ifr_name);

    /* this is the special file descriptor that the caller will use to talk
     * with the virtual interface */
    return fd;
}

void tun_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *nlh)
{
    int offset = 0, next_header = 41;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr;
    int pkt_len, err;
    char *pkt_data;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);


    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    //hexdump(pkt_data + offset, pkt_len - offset);
    //puts("");

    err = write(fd_tun, pkt_data + offset, pkt_len - offset);
    if (err < 0) {
        perror("Writing data");
        exit(1);
    }
}

int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused)
{
    return NL_SKIP;
}

void usage(char *av0)
{
    fprintf(stderr,
            """Usage: %s [-v] [--memory mb] [--interface name] ip_segment\n \
            MODE :=          bulk|debulk                                        \n \
            --memory         memory in Mb available for the socket (default=128)\n \
            --interface      name of the interface to be created\n""", av0);
    exit(-1);
}


int main(int argc, char **argv)
{
    struct seg6_sock *sk;

    struct in6_addr in6;
    int ret, c, memory = 1024, nb_threads = 0;

    char *ip6_binding = malloc(INET6_ADDRSTRLEN * sizeof(char));


    // Get opt http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    while (1) {
        static struct option long_options[] =
                {
                        {"verbose", no_argument      , NULL         , 'v'   },
                        {"memory" , required_argument, NULL         , 'm'   },
                        {"interface" , required_argument, NULL         , 'i'   },
                        {"threads", required_argument, NULL, 'n'},
                        {0        , 0                , 0            , 0     }
                };
        int option_index = 0;
        c = getopt_long (argc, argv, "v", long_options, &option_index);

        if (c == -1) // Detect the end of the options.
            break;

        switch (c) {
            case 0: // If this option set a flag, do nothing else now.
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'v': verbose_flag = 1;          break;
            case 'm': memory = atoi(optarg);     break;
            case 'n': nb_threads = atoi(optarg); break;
            case 'i': strncpy(interface_name, optarg, 256); break;
            default : usage(argv[0]);
        }
    }

    if (optind >= argc || optind+1 < argc) {
        usage(argv[0]);
        exit(0);
    }

    if (verbose_flag){
        puts("verbose flag is set");
        printf("Memory allocated for the socket : %d\n", memory);
    }


    strcpy(ip6_binding, argv[optind]);
    inet_pton(AF_INET6, ip6_binding, &in6);

    /*
     * seg6_socket_create(block_size, block_nr)
     * mem usage = block_size * block_nr * 2
     * default settings = 8MB usage for 4K pages
     * increase block_size and not block_nr if needed
     */

    sk = seg6_socket_create(2048*getpagesize(), 64);

    fd_tun = tun_alloc(interface_name, IFF_TUN | IFF_NO_PI);

    if(ioctl(fd_tun, TUNSETPERSIST, 0) < 0){
        perror("disabling TUNSETPERSIST");
        exit(1);
    }

    if (verbose_flag){
        printf("Tunnel %s created\n", interface_name);
    }

    ret = asynchrone_service_launch(sk, &in6, nl_recv_ack, tun_in, nb_threads);

    if (ret)
        fprintf(stderr, "seg6_send_and_recv(): %s\n", strerror(-ret));

    seg6_socket_destroy(sk);
    free(ip6_binding);

    return 0;
}
