#include "compression_lz4.h"
#include "fake_data.h"

extern int compression_level;
extern int verbose_flag;
extern int mtu_buffer;


void compress_lz4_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, res, bound, offset = 0, next_header = 41;
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    unsigned int in_len, out_len = 0;
    unsigned char* pkt_data_compressed;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Compress data
    in_len  = pkt_len-offset;
    bound = LZ4_compressBound(in_len);

    pkt_data_compressed = malloc(offset + bound);

    // compress
    out_len = LZ4_compress_fast(pkt_data + offset, pkt_data_compressed + offset, in_len, bound, compression_level);


    int n_pkt_len = offset+out_len;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_compressed, pkt_data, offset);

    if(verbose_flag) printf("packet size %d -> %d\n", pkt_len, n_pkt_len);


    // Send message
#if FAKE_LZ4 == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_compressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif

    free(pkt_data_compressed);
}


void decompress_lz4_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, uncompressed_size, offset = 0, next_header = 41;
    unsigned char* pkt_data_decompressed = malloc(mtu_buffer);
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    uint16_t in_len;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

#if FAKE_LZ4 == 0
    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = pkt_len-offset;

    // decompress
    //uint16_t n_pkt_len = 0;
    //memcpy(&n_pkt_len, pkt_data + offset, 2);
    //res = LZ4_uncompress_unknownOutputSize(pkt_data + offset, pkt_data_decompressed + offset, in_len, mtu_buffer);

    uncompressed_size = LZ4_decompress_safe(pkt_data + offset, pkt_data_decompressed + offset, in_len, mtu_buffer - offset);
#else
    offset += nparse_ipv6(packet_bytes_lz4, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(packet_bytes_lz4, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = sizeof(packet_bytes_lz4)-offset;

    // decompress
    //uint16_t n_pkt_len = 0;
    //memcpy(&n_pkt_len, pkt_data + offset, 2);
    //res = LZ4_uncompress_unknownOutputSize(pkt_data + offset, pkt_data_decompressed + offset, in_len, mtu_buffer);

    uncompressed_size = LZ4_decompress_safe(packet_bytes_lz4 + offset, pkt_data_decompressed + offset, in_len, mtu_buffer - offset);
#endif


    // Modify packet length according to compressed_length
    int n_pkt_len = offset + uncompressed_size;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_decompressed, pkt_data, offset);

    //hexdump(pkt_data_decompressed+offset, n_pkt_len-offset-2);

    if(verbose_flag) printf("packet size %d -> %d\n", pkt_len, n_pkt_len);


    // Send message
#if FAKE_LZ4 == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_decompressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif
    free(pkt_data_decompressed);
}

int init_lz4(){
    return 0;
}
