#include "compression_lzo.h"
#include "fake_data.h"

extern int compression_level;
extern int verbose_flag;
extern int mtu_buffer;

void compress_lzo_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, res, offset = 0, next_header = 41;
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    lzo_uint in_len, out_len = 0;
    unsigned char* pkt_data_compressed;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Compress data
    in_len  = pkt_len-offset;

    pkt_data_compressed = malloc(offset + in_len + in_len / 16 + 64 + 3); // According to the documentation

    // compress
    lzo1x_1_compress( (unsigned char *)(pkt_data+offset), in_len, pkt_data_compressed + offset, &out_len, wrkmem);


    int n_pkt_len = offset+out_len;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_compressed, pkt_data, offset);

    if(verbose_flag) printf("flow %d : packet size %d -> %d\n", (((ntohs(ip6hdr->version_class_flow) << 16 ) | (ntohs(ip6hdr->version_class_flow) >> 16)) & 0xfffff), pkt_len, n_pkt_len);

    // Send message
#if FAKE_LZO == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_compressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif

    free(pkt_data_compressed);
}


void decompress_lzo_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, offset = 0, next_header = 41;
    unsigned char* pkt_data_decompressed = malloc(mtu_buffer);
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    lzo_uint in_len, uncompressed_size;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

#if FAKE_LZO == 0
    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = pkt_len-offset;

    // decompress
    lzo1x_decompress( (unsigned char *)(pkt_data+offset), in_len, pkt_data_decompressed+offset, &uncompressed_size, wrkmem);
#else
    offset += nparse_ipv6(packet_bytes_lzo, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(packet_bytes_lzo, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = sizeof(packet_bytes_lzo) - offset;

    // decompress
    lzo1x_decompress( (unsigned char *)(packet_bytes_lzo+offset), in_len, pkt_data_decompressed+offset, &uncompressed_size, wrkmem);
#endif

    // Modify packet length according to compressed_length
    int n_pkt_len = offset + uncompressed_size;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_decompressed, pkt_data, offset);

    //hexdump(pkt_data_decompressed+offset, n_pkt_len-offset);

    if(verbose_flag) printf("packet size %d -> %d\n", pkt_len, n_pkt_len);


    // Send message
#if FAKE_LZO == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_decompressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif

    free(pkt_data_decompressed);
}

int init_lzo(){
    if (lzo_init() != LZO_E_OK) {
        printf("internal error - lzo_init() failed !!!\n");
        printf("(this usually indicates a compiler bug - try \
            recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' \
            for diagnostics)\n");
        return 3;
    }
    return 0;
}