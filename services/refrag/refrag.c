
#include "refrag.h"
#include <openssl/md5.h>

extern int verbose_flag;


long TIMEOUT = 5000;

struct bulk_frag *frag_packets = NULL;
pthread_mutex_t hashlist_mutex = PTHREAD_MUTEX_INITIALIZER;


void* pkt_timeout (void *args) {
    struct seg6_sock *sk = ((struct thread_timeout_args *) args)->sk;
    struct bulk_frag *current_b, *tmp;
    struct timeval now;

    while(1) {
        usleep(TIMEOUT*1000);

        if(pthread_mutex_lock(&hashlist_mutex) != 0) fprintf(stderr, "cannot lock struct mutex");

        gettimeofday(&now, NULL);

        HASH_ITER(hh, frag_packets, current_b, tmp){

            if(timevaldiff(&(current_b->first_received), &now) >= TIMEOUT){
                if(verbose_flag) printf("Freeing bulk id : %u\n", current_b->id);

                if(current_b->buffer != NULL)
                    free(current_b->buffer);
                if(current_b->lock != NULL) {
                    pthread_mutex_destroy(current_b->lock);
                    free(current_b->lock);
                }
                HASH_DEL(frag_packets, current_b);
                free(current_b);
            }

        }

        if(pthread_mutex_unlock(&hashlist_mutex) != 0) fprintf(stderr, "cannot unlock struct mutex");

    }
}


void add_to_frags(char *pkt_data, int offset, struct bulk_frag *frags, uint16_t frag_offset, size_t frag_size){
    int offset_pkt_frag = offset - IP6HDR_FRAG_SIZE;
    size_t write_frag_offset = 8 * frag_offset + offset_pkt_frag;

    if(frag_offset == 0) { // first frag
        memcpy(frags->buffer, pkt_data, offset_pkt_frag); // copy into buffer with IPv6 header

        gettimeofday(&(frags->first_received), NULL); // Setup the reception time
        frags->wrote_bytes += offset_pkt_frag;

        if(verbose_flag) printf("\tWrote %u bits @ %u\n", offset_pkt_frag, 0);
    }

    // following frags
    memcpy(frags->buffer + write_frag_offset, pkt_data + offset, frag_size); // copy frags into buffer
    frags->wrote_bytes += frag_size; // update status
    if(verbose_flag) printf("\tWrote %u bits @ %u\n", frag_size, write_frag_offset);
}

void send_frags(struct seg6_sock *sk, int offset, struct bulk_frag *frags){
    int offset_pkt_frag = offset - IP6HDR_FRAG_SIZE;
    struct parser_ipv6 *inner_ip6;

    verbose("Sending re-fragmented packet\n");
    // find inner IPv6
    find_hdr_before(frags->buffer, NEXTHDR_FRAGMENT, NEXTHDR_IPV6, NULL, &inner_ip6, NEXTHDR_IPV6);

    // change attrs (next_header and length)
    inner_ip6->next_header = frags->next_header;
    inner_ip6->length = htons(frags->end_pos - offset_pkt_frag);

    // change outer IPv6 length
    ((struct parser_ipv6*)frags->buffer)->length = htons(frags->end_pos - (uint16_t)IP6HDR_SIZE);

    send_ip6_packet(sk, (int) frags->end_pos, frags->buffer);
}

void refrag_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    int pkt_len, offset=0, next_header = NEXTHDR_IPV6, tmp_nh;
    size_t end_pos;
    char *pkt_data;
    struct parser_ipv6_frag *ip6hdr_frag;

    //printf("packet in\n");

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    // Find fragmentation
    offset += parse_until(NEXTHDR_FRAGMENT, pkt_data, offset, &next_header);

    if (offset == -1){
        verbose("Not able to find Fragment header\n");
        return;
    }

    offset += nparse_ipv6_frag(pkt_data, &ip6hdr_frag, offset, &next_header);

    uint8_t frag_m_flag = ip6hdr_frag->end_frag_offset_res_m & 1;
    uint32_t frag_id = ntohl(ip6hdr_frag->identification);
    uint16_t frag_offset = (ip6hdr_frag->start_frag_offset << 5) | (ip6hdr_frag->end_frag_offset_res_m >> 3);
    size_t frag_size = pkt_len - offset;

    if(verbose_flag) printf("m: %u - id: %u - frag_offset : %u - size : %d - offset: %d\n", frag_m_flag, frag_id, frag_offset, frag_size, offset);

    pthread_mutex_lock(&hashlist_mutex);
    verbose("Lock struct mutex\n");
    // Retrieve buffer
    struct bulk_frag *frags = get_bulk_frag_or_create((struct bulk_frag **) &frag_packets, frag_id);

    pthread_mutex_lock(frags->lock);
    verbose("Lock buffer mutex\n");

    end_pos = 8 * frag_offset + frag_size + offset - IP6HDR_FRAG_SIZE;

    if(end_pos > BUFFER_FRAGS_SIZE){
        verbose("re-fragmented packet will be bigger than the buffer (MTU), fragment dropped.\n");
    } else {

        add_to_frags(pkt_data, offset, frags, frag_offset, frag_size);

        if(frag_m_flag == 0){ // last frag, the end of the frag is known now
            frags->end_pos = end_pos;
        }

        if(frag_offset == 0){ // first frag, save nh for refrag
            frags->next_header = next_header;
        }

        if(frags->wrote_bytes >= frags->end_pos){
            send_frags(sk, offset, frags);
            if(verbose_flag) printf("Freeing bulk id : %u\n", frags->id);

            if(frags->buffer != NULL)
                free(frags->buffer);
            if(frags->lock != NULL) {
                pthread_mutex_destroy(frags->lock);
                free(frags->lock);
            }
            HASH_DEL(frag_packets, frags);
            free(frags);

        } else {
            verbose("... Unlock mutex\n");
            pthread_mutex_unlock(frags->lock);
        }
    }

    verbose("Unlock struct mutex\n");
    pthread_mutex_unlock(&hashlist_mutex);
}


int nl_recv_ack(struct nlmem_sock *nlm_sk __unused, struct nlmsghdr *hdr __unused, void *arg __unused) {
    return NL_SKIP;
}

void usage(char *av0)
{
    fprintf(stderr, """Usage: %s ip_segment [options]\n\
OPTIONS:\n\
    -B, --buffer    size \n\
        The size in byte to use for the buffer (maximum MTU refragmentation)\n\
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
    int ret, c, memory = 128, i;

    struct thread_timeout_args thread_arg;
    pthread_t thread;

    char *ip6_binding = malloc(INET6_ADDRSTRLEN * sizeof(char));

    static struct option long_options[] =
            {
                    {"verbose", no_argument      , NULL         , 'v'   },
                    {"memory" , required_argument, NULL         , 'M'   },
                    {"buffer" , required_argument, NULL         , 'B'   },
                    {"delta"  , required_argument, NULL         , 'D'   },
                    {0        , 0                , 0            , 0     }
            };

    // Get opt http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    //while ((c = getopt_long (argc, argv, "vbdm:t:", long_options, &option_index)) != 1) {
    while (1) {
        int option_index = 0;
        c = getopt_long (argc, argv, "vm:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 0:
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 'M':
                memory = atoi(optarg);
                break;
            case 'B':
                BUFFER_FRAGS_SIZE = atoi(optarg);
                break;
            case 'D':
                TIMEOUT = atoi(optarg);
                break;
            default : usage(argv[0]);
        }
    }

    if (optind >= argc || optind+1 < argc) {
        usage(argv[0]);
        exit(0);
    }

    if (verbose_flag)
        printf("Verbose flag is set\nMemory allocated for the socket : %d\n", memory);

    strcpy(ip6_binding, argv[optind]);
    inet_pton(AF_INET6, ip6_binding, &in6);

    /*
     * seg6_socket_create(block_size, block_nr)
     * mem usage = block_size * block_nr * 2
     * default settings = 8MB usage for 4K pages
     * increase block_size and not block_nr if needed
     */

    sk = seg6_socket_create(memory*getpagesize(), 64);

    thread_arg.sk = sk;

    pthread_create(&thread, NULL, &pkt_timeout, (void *) &thread_arg);

    ret = synchrone_service_launch(sk, &in6, nl_recv_ack, refrag_packet, 0);

    pthread_join(thread, NULL);

    if (ret)
        fprintf(stderr, "seg6_send_and_recv(): %s\n", strerror(-ret));

    seg6_socket_destroy(sk);

    return 0;
}
