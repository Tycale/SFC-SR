
#include "bulk_buffer.h"


size_t BUFFER_SIZE = 5000;

extern int verbose_flag;


int get_tcp_info(struct nlattr **attrs, struct tcp_process *tcp_info,
        struct parser_tcp **tcphdr, struct parser_ipv6 **ip6hdr, unsigned int *hash) {
    int res;

    tcp_info->pkt_len = (int) nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    tcp_info->pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    if(find_hdr_before_compute(tcp_info->pkt_data, NEXTHDR_TCP,
                    NEXTHDR_IPV6, tcphdr, ip6hdr, NEXTHDR_IPV6, hash) != 0)
        return -1;

    tcp_info->tcp_offset = (void*)*tcphdr-(void*)tcp_info->pkt_data;
    tcp_info->tcp_len = nparse_tcp(*tcphdr, NULL, 0, NULL);

    // FIXME
    tcp_info->opt = DJBHash(((char *) *tcphdr)+20, tcp_info->tcp_len-20);
    //tcp_info->opt = hashlittle(((unsigned char *) *tcphdr)+20, tcp_info->tcp_len-20, 0);
    //MD5(((unsigned char *) *tcphdr)+20, tcp_info->tcp_len-20, tcp_info->opt);

    return 0;
}

void update_tcp_hdr(struct bulk_buffer *buffer, struct tcp_process *tcp_info, uint32_t *seq) {

    struct parser_tcp *bulked, *pkt;
    struct parser_ipv6 *ip6hdr;

    int hdr_len = tcp_info->tcp_len + tcp_info->tcp_offset;

    pkt =(struct parser_tcp *) (tcp_info->pkt_data + tcp_info->tcp_offset);
    bulked =(struct parser_tcp *) ((buffer->buffer) + tcp_info->tcp_offset);

    ip6hdr =(struct parser_ipv6 *) (buffer->buffer + tcp_info->tcp_offset - 40);

    ip6hdr->length = htons(buffer->pos - tcp_info->tcp_offset);

    //printf("ack: %u",ntohl(bulked->ack_nbr));
    bulked->ack_nbr = pkt->ack_nbr;


    if(( get_tcp_flag(pkt, FLAG_ACK) && ! get_tcp_flag(bulked, FLAG_ACK) ) ||
            get_tcp_flag(pkt, FLAG_ACK) && get_tcp_flag(bulked, FLAG_ACK) &&
            ntohl(pkt->ack_nbr) > ntohl(bulked->ack_nbr))
        bulked->ack_nbr = pkt->ack_nbr;

    bulked->len_res_control =  htons((ntohs(bulked->len_res_control)
                | (ntohs(pkt->len_res_control) & 0x1FF)));
}


//TODO clean
void insert_tcp_to_buffer(struct seg6_sock *sk, struct bulk_buffer *buffer,
        struct tcp_process *tcp_info, uint32_t *seq, uint32_t *opt) {

    int offset = tcp_info->tcp_offset;
    int hdr_len = tcp_info->tcp_len + tcp_info->tcp_offset;
    int pkt_len = tcp_info->pkt_len;
    char *pkt_data = tcp_info->pkt_data;

    int md5_cmp = 1;

    struct parser_tcp *tcphdr = (struct parser_tcp *) (tcp_info->pkt_data + tcp_info->tcp_offset);

    if(buffer->pos != 0 && (
                *opt != tcp_info->opt ||
                //(md5_cmp = strncasecmp(opt, tcp_info->opt, JENKIN_LEN)) != 0 ||
                available_space(buffer, hdr_len) < pkt_len - hdr_len ||
                *seq != ntohl(tcphdr->seq_nbr))
            ) {
        // FIXME add flags
        send_buffer(sk, buffer, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM | OP_BUF_UPDLEN);
        *opt = tcp_info->opt;
    }

    if(buffer->pos == 0) {
        add_to_buffer(buffer, pkt_data, pkt_len);
        *seq = (ntohl(tcphdr->seq_nbr) + tcp_info->pkt_len - hdr_len) % TCP_MAX_SEG;
        gettimeofday(&(buffer->last_send), NULL);
    } else {
        add_to_buffer(buffer, pkt_data+hdr_len, pkt_len - hdr_len);
        *seq = (*seq + tcp_info->pkt_len - hdr_len) % TCP_MAX_SEG;
        update_tcp_hdr(buffer, tcp_info, seq);
    }
    //printf("New seq: %ud\n", *seq);

}


/*
 *
 */
void send_buffer(struct seg6_sock *sk, struct bulk_buffer *buffer, int flag) {
    int offset=0, next_header=NEXTHDR_IPV6, tcp_len;
    struct parser_tcp *tcphdr ;
    struct parser_ipv6 *ip6hdr ;

    if(buffer->pos == 0)
        return;

    if(buffer->nbr == 1)
        goto send;

    if(flag & OP_BUF_UPDLEN) {
        if( flag & OP_BUF_UPDALL_LEN) {
            int tmp=0;
            offset=0;

            while((offset = find_hdr(NEXTHDR_IPV6,
                            buffer->buffer + tmp,
                            &ip6hdr,
                            &next_header)) >= 0 && tmp < (int) buffer->pos) {
                tmp += offset + IP6HDR_SIZE;
                ip6hdr->length = htons(buffer->pos - (uint16_t) tmp);
            }
        } else {
            ip6hdr = (struct parser_ipv6 *) buffer->buffer;
            ip6hdr->length = htons(buffer->pos - (uint16_t)IP6HDR_SIZE);
        }
    }

    if(flag & OP_BUF_UPDCKSUM) {
        find_hdr_before(buffer->buffer, NEXTHDR_TCP, NEXTHDR_IPV6, &tcphdr, &ip6hdr, NEXTHDR_IPV6);
        set_tcp_checksum(ip6hdr, tcphdr, buffer->pos -
                ((void *) tcphdr - (void *)buffer->buffer));
    }

send:
    if(verbose_flag)
        puts("Sending bulk buffer");

#if FAKE_HULK == 0
    send_ip6_packet(sk, buffer->pos, buffer->buffer);
#endif

    buffer->pos = 0;
    buffer->nbr = 0;
}


void send_old_buffer(struct seg6_sock *sk, struct bulk_buffer *buffer, int ms_delta, int flag) {
    struct timeval now;

    //FIXME
    pthread_mutex_lock(buffer->lock);
    //if(pthread_mutex_trylock(buffer->lock) == 0) {
        gettimeofday(&now, NULL);
        if(buffer->pos > 0 && timevaldiff(&(buffer->last_send), &now) >= ms_delta){
            //printf("thread sent\n");
            send_buffer(sk, buffer, flag);
        }

        pthread_mutex_unlock(buffer->lock);
    //}
}


/*
 * @pre: Buffer must contain have at least pkt_len available space in order to
 * contains pkt_data.
 * @return: number of byte write in buffer
 */
int add_to_buffer(struct bulk_buffer *b, const char *pkt_data, size_t pkt_len) {

    if(verbose_flag)
        printf("Add %d to offset %d\n", pkt_len, b->pos);

    memcpy((b->buffer+(b->pos)), pkt_data, pkt_len);

    // Update
    if(b->nbr == 0)
        gettimeofday(&(b->last_send), NULL);

    b->pos = b->pos + pkt_len;
    b->nbr += 1;

    if(verbose_flag)
        printf("\tnew offset = %d\n", b->pos);


    return pkt_len;
}

/*
 * @return: the available space
 */
int available_space(struct bulk_buffer *b, size_t offset) {
    if(verbose_flag)
        printf("Available: %d\n", BUFFER_SIZE - ((b->pos == 0)? offset : b->pos ));
    return BUFFER_SIZE - ((b->pos == 0)? offset : b->pos );
}

/*
 * Init complete bulk_buffer
 * @return: 0 if no error, -1 else
 */
int init_bulk_buffer(struct bulk_buffer **b, size_t size) {
    if((*b = (struct bulk_buffer *) malloc(sizeof(struct bulk_buffer))) == NULL) {
        fprintf(stderr, "bulk_buffer cannot be malloc\n");
        return -1;
    }

    (*b)->buffer = NULL;
    (*b)->lock = NULL;

    if( ((*b)->buffer = (char *) malloc(size)) == NULL ){
        fprintf(stderr, "Unable to malloc buffer\n");
        free_bulk_buffer(*b);
        return -1;
    }

    if(((*b)->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t))) == NULL) {
        fprintf(stderr, "Unable to allow size for lock\n");
        free_bulk_buffer(*b);
        return -1;
    }

    if(pthread_mutex_init((*b)->lock, NULL) != 0) {
        fprintf(stderr, "Unable to init lock\n");
        free_bulk_buffer(*b);
        return -1;
    }

    (*b)->pos = 0;
    (*b)->nbr = 0;

    return 0;
}

/*
 * Free bulk_buffer
 */
void free_bulk_buffer(struct bulk_buffer *b) {
    if(b != NULL){
        if(b->buffer != NULL)
            free(b->buffer);
        if(b->lock != NULL) {
            pthread_mutex_destroy(b->lock);
            free(b->lock);
        }
    }

    free(b);
}


