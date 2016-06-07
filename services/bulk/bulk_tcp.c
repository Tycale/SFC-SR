
#include "bulk_tcp.h"


extern char hash[TCP_HASH_SIZE+1];

extern void *hash_flows2;

void bulk_tcp(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    struct parser_tcp *tcphdr ;
    struct parser_ipv6 *ip6hdr ;
    struct tcp_flow_2 *flow2;
    struct bulk_buffer *buffer;
    struct tcp_process tcp_info;
    int res;
    unsigned int ipv6_hash=0, tmp, *pos;

    if((res = get_tcp_info(attrs, &tcp_info, &tcphdr, &ip6hdr, &ipv6_hash)) != 0){
        send_ip6_packet(sk,
                (int) nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]),
                nla_data(attrs[SEG6_ATTR_PACKET_DATA]));
        return ;
    }

    if(tcp_info.pkt_len >= (int) BUFFER_SIZE || tcphdr == NULL || ip6hdr == NULL) {
        send_ip6_packet(sk, tcp_info.pkt_len, tcp_info.pkt_data);
        return ;
    }

    if(get_tcp_flag(tcphdr, FLAG_URG | FLAG_SYN | FLAG_RST) ) {
         send_ip6_packet(sk, tcp_info.pkt_len, tcp_info.pkt_data);
         return ;
    }

    // Retreive buffer
    pos = (unsigned int *) ((char *) ip6hdr + offsetof(struct parser_ipv6, length));

    tmp = *pos;
    *pos = ipv6_hash;
    flow2 = get_tcp_flow_2_or_create((struct tcp_flow_2 **) &hash_flows2,
            (char *) pos, (char *) &(tcphdr->src));
    *pos = tmp;
    buffer = flow2->b;


    pthread_mutex_lock(buffer->lock);

    insert_tcp_to_buffer(sk, buffer, &tcp_info, &(flow2->seq), &(flow2->opt));

    if(get_tcp_flag(tcphdr, FLAG_FIN | FLAG_PSH) )
        send_buffer(sk, buffer, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM | OP_BUF_UPDALL_LEN);

    pthread_mutex_unlock(buffer->lock);

}





void bulk_tcp_list(struct seg6_sock *sk, struct nlattr **attrs, struct nlmsghdr *nlh) {
    struct parser_tcp *tcphdr ;
    struct parser_ipv6 *ip6hdr;
    struct tcp_flow_list_2 *flow2;
    struct bulk_list *list;
    struct el_bulk_list *el, *pivot;
    struct tcp_process *tcp_info;
    struct timeval a, b, c, d;
    int ipv6_hash=0;


    tcp_info = (struct tcp_process *) malloc(sizeof(struct tcp_process));
    get_tcp_info(attrs, tcp_info, &tcphdr, &ip6hdr, &ipv6_hash);

    if(tcp_info->pkt_len >= (int) BUFFER_SIZE || tcphdr == NULL || ip6hdr == NULL) {
        send_ip6_packet(sk, tcp_info->pkt_len, tcp_info->pkt_data);
        free(tcp_info);
        return;
    }

    if(get_tcp_flag(tcphdr, FLAG_URG | FLAG_SYN | FLAG_RST) ) {
         send_ip6_packet(sk, tcp_info->pkt_len, tcp_info->pkt_data);
         free(tcp_info);
         return ;
    }

    // FIXME
    // gettimeofday(&a,  NULL);
    // Retrieve flow and buffer
//    memcpy(hash, &tcphdr->src, 4 );
//    memcpy(hash+4, &ip6hdr->src, 32 );
//    hash[TCP_HASH_SIZE] = '\0';
//    flow = get_tcp_flow_list_or_create((struct tcp_flow_list **) &hash_flows, (char *) hash);
//
//    gettimeofday(&b,  NULL);
//
//    gettimeofday(&c,  NULL);
    flow2 = get_tcp_flow_list_2_or_create((struct tcp_flow_list_2 **) &hash_flows2,
            (char *) &(ip6hdr->src), (char *) &(tcphdr->src));

    //gettimeofday(&d,  NULL);

    //printf("First: %f, second: %f \n", timevaldiff_double(&a, &b), timevaldiff_double(&c, &d));
    //
    //printf("flow: %p, flow2: %p\n", flow, flow2);


    // FIXME
    //list = flow->b;
    list = flow2->b;


    el = (struct el_bulk_list *) malloc(sizeof(struct el_bulk_list));
    el->tcp = tcp_info;
    el->seq = ntohl(tcphdr->seq_nbr);
    // FIXME
    el->len = el->tcp->pkt_len - el->tcp->tcp_offset - el->tcp->tcp_len;

    //printf("SEQ: %u\n", el->seq);
    //printf("tcp len: %u\n", el->len);

    pthread_mutex_lock(list->lock);

    pivot = insert_tcp_to_list(sk, list, el);
    //show_list(list->list);

    if(get_tcp_flag(tcphdr, FLAG_FIN | FLAG_PSH) ) {
        //printf("SEND\n");
        send_list(sk, list, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM | OP_BUF_UPDALL_LEN);
    } else if(pivot != NULL) {
        //printf("yeah\n");
        send_list_until(sk, list, pivot->next, OP_BUF_UPDLEN | OP_BUF_UPDCKSUM | OP_BUF_UPDALL_LEN );
    }
    //show_list(list->list);

    pthread_mutex_unlock(list->lock);
}
