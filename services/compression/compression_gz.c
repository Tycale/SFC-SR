#include "compression_gz.h"
#include "fake_data.h"

extern int compression_level;
extern int verbose_flag;
extern int mtu_buffer;

void compress_gz_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, res, offset = 0, next_header = 41;
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    unsigned int in_len;
    unsigned char* pkt_data_compressed;

    uLongf out_len = 0;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Compress data
    in_len  = pkt_len-offset;

    out_len = compressBound(in_len);
    pkt_data_compressed = malloc(offset + out_len);

    // compress
    compress2(pkt_data_compressed + offset, &out_len, pkt_data + offset, in_len, compression_level);

    int n_pkt_len = offset+out_len;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_compressed, pkt_data, offset);

    if(verbose_flag) printf("packet size %d -> %d\n", pkt_len, n_pkt_len);


    // Send message
#if FAKE_GZ == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_compressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif

    free(pkt_data_compressed);
}


void decompress_gz_packet_in(struct seg6_sock *sk __unused, struct nlattr **attrs, struct nlmsghdr *msghdr __unused)
{
    int pkt_len, offset = 0, next_header = 41;
    unsigned char* pkt_data_decompressed = malloc(mtu_buffer);
    char *pkt_data;

    struct nlmsghdr *msg;
    struct parser_ipv6 *ip6hdr ;
    struct parser_ipv6_sr *ip6hdr_sr ;

    unsigned long in_len;
    uLongf uncompressed_size;

    // Get packet length and data
    pkt_len = nla_get_u32(attrs[SEG6_ATTR_PACKET_LEN]);
    pkt_data = nla_data(attrs[SEG6_ATTR_PACKET_DATA]);

#if FAKE_GZ == 0
    // Parse header length to have the offset of data
    offset += nparse_ipv6(pkt_data, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(pkt_data, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = pkt_len-offset;

    uncompressed_size = (unsigned long) mtu_buffer - offset;

    // decompress
    uncompress(pkt_data_decompressed + offset, &uncompressed_size, pkt_data + offset, in_len);
#else
    // Parse header length to have the offset of data
    offset += nparse_ipv6(packet_bytes_gz, &ip6hdr, offset, &next_header);
    offset += nparse_ipv6_sr(packet_bytes_gz, &ip6hdr_sr, offset, &next_header);

    // Decompress data
    in_len  = sizeof(packet_bytes_gz) - offset;

    uncompressed_size = (unsigned long) mtu_buffer - offset;

    // decompress
    uncompress(pkt_data_decompressed + offset, &uncompressed_size, packet_bytes_gz + offset, in_len);
#endif

    // Modify packet length according to compressed_length
    int n_pkt_len = offset + uncompressed_size;
    ip6hdr->length = htons(n_pkt_len-(uint16_t)40); // 40 for IPv6 header
    memcpy(pkt_data_decompressed, pkt_data, offset);

    //hexdump(pkt_data_decompressed+offset, n_pkt_len-offset-2);

    if(verbose_flag) printf("packet size %d -> %d\n", pkt_len, n_pkt_len);


    // Send message
#if FAKE_GZ == 0
    send_ip6_packet(sk, n_pkt_len, pkt_data_decompressed);
#else
    ip6hdr->length = htons(pkt_len-(uint16_t)40);
    send_ip6_packet(sk, pkt_len, pkt_data);
#endif

    free(pkt_data_decompressed);
}


int init_gz(){
    return 0;
}
