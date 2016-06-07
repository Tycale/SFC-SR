#include "bulk_pkt.h"

extern void *hash_flows;
extern void *hash_flows2;

/*
 *
 */
void send_debulk_buffer(struct seg6_sock *sk, struct parser_ipv6_sr *bulk_sr, int debulk_len, char *debulk_data, int bulk_hdr_size) {
    int len, next_header = 41, offset = 0;
    struct parser_ipv6 *ip6hdr;
    struct parser_ipv6_sr *ip6hdr_sr;

    verbose("Sending debulk buffer\n");

    while (offset < debulk_len) {
        if(verbose_flag)
            printf("Sending packet \n\
                    read from offset %d\n", offset+bulk_hdr_size);

        nparse_ipv6(debulk_data, &ip6hdr, offset, &next_header);
        nparse_ipv6_sr(debulk_data, &ip6hdr_sr, offset+IP6HDR_SIZE, &next_header);

        ip6hdr_sr->seg_left = bulk_sr->seg_left;
        len = ntohs(ip6hdr->length) + IP6HDR_SIZE;

        send_ip6_packet(sk, len, debulk_data+offset);
        offset += len;

        if(verbose_flag)
            printf("new offset = %d\n\
                ... Finish\n", offset);
    }

    if(verbose_flag)
        printf("... Finish\n");
}


/*
 *
 */
void debulk_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    int pkt_len, offset = 0, next_header = 41;
    char *pkt_data;
    struct parser_ipv6 *ip6hdr;
    struct parser_ipv6_sr *ip6hdr_sr;

    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    send_debulk_buffer(sk, ip6hdr_sr, pkt_len-offset, pkt_data+offset, offset);
}


/*
 *
 */
void bulk_packet(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    int pkt_len, flow_id=0, offset = 0, next_header = NEXTHDR_IPV6;
    char *pkt_data;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr;

    struct bulk_flow *flow;
    struct bulk_buffer *buffer;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);
    offset = find_hdr(NEXTHDR_ROUTING, pkt_data, &ip6hdr_sr, &next_header);
    offset += 8 + 8 * ip6hdr_sr->hdr_length;


    if(pkt_len >= (int) BUFFER_SIZE) {
        fprintf(stderr, "Drop too big packet");
        return;
    }

    if( pkt_data + 40 != (char *) ip6hdr_sr ||
            ip6hdr_sr->next_header != NEXTHDR_IPV6 ) {

        send_buffer(sk, buffer, OP_BUF_UPDLEN );
        add_to_buffer(buffer, pkt_data, offset);
        add_to_buffer(buffer, pkt_data, pkt_len);
        return;
    }

    // Retrieve flow and buffer
    flow_id = (int) do_csum(ip6hdr_sr + 8, 8 * (ip6hdr_sr->hdr_length));
    flow = get_bulk_flow_or_create((struct bulk_flow **) &hash_flows2, flow_id);
    buffer = flow->b;

    pthread_mutex_lock(buffer->lock);
    verbose("Lock mutex\n");

    if(available_space(buffer, offset) < pkt_len)
        send_buffer(sk, buffer, OP_BUF_UPDLEN );

    // Add first header
    if( buffer->pos == 0 ) {
        verbose("Update pos to offset because no header\n");
        add_to_buffer(buffer, pkt_data, offset);
        gettimeofday(&(flow->b->last_send), NULL);
    }

    add_to_buffer(buffer, pkt_data, pkt_len);

    verbose("... Unlock mutex\n");
    pthread_mutex_unlock(buffer->lock);

#if FAKE_HULK
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif
}

