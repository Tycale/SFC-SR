
#include "checksum.h"

#ifndef __CHECKSUM__T
#define __CHECKSUM__T

#ifndef csum_fold
/*
 * Fold a partial checksum
 */
inline __sum16 csum_fold(__wsum csum)
{
	__u32 sum = (__u32)csum;
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return (__sum16)~sum;
}
#endif



#ifndef do_csum
inline unsigned short from32to16(unsigned int x)
{
	/* add up 16-bit and 16-bit for 16+c bit */
	x = (x & 0xffff) + (x >> 16);
	/* add up carry.. */
	x = (x & 0xffff) + (x >> 16);
	return x;
}

unsigned int do_csum(const unsigned char *buff, int len)
{
	int odd;
	unsigned int result = 0;

	if (len <= 0)
		goto out;
	odd = 1 & (unsigned long) buff;
	if (odd) {
#ifdef __LITTLE_ENDIAN
		result += (*buff << 8);
#else
		result = *buff;
#endif
		len--;
		buff++;
	}
	if (len >= 2) {
		if (2 & (unsigned long) buff) {
			result += *(unsigned short *) buff;
			len -= 2;
			buff += 2;
		}
		if (len >= 4) {
			const unsigned char *end = buff + ((unsigned)len & ~3);
			unsigned int carry = 0;
			do {
				unsigned int w = *(unsigned int *) buff;
				buff += 4;
				result += carry;
				result += w;
				carry = (w > result);
			} while (buff < end);
			result += carry;
			result = (result & 0xffff) + (result >> 16);
		}
		if (len & 2) {
			result += *(unsigned short *) buff;
			buff += 2;
		}
	}
	if (len & 1)
#ifdef __LITTLE_ENDIAN
		result += *buff;
#else
		result += (*buff << 8);
#endif
	result = from32to16(result);
	if (odd)
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
	return result;
}
#endif



unsigned int csum_partial(const void *buff, int len, unsigned int wsum)
{
	unsigned int sum = wsum;
	unsigned int result = do_csum(buff, len);

	/* add in old sum, and carry.. */
	result += sum;
	if (sum > result)
		result += 1;
	return result;
}


#ifndef _HAVE_ARCH_IPV6_CSUM
__sum16 csum_ipv6_magic(const struct in6_addr *saddr,
			const struct in6_addr *daddr,
			__u32 len, unsigned short proto,
			__wsum csum)
{

	int carry;
	__u32 ulen;
	__u32 uproto;
	__u32 sum = (__u32)csum;

	sum += (__u32)saddr->s6_addr32[0];
	carry = (sum < (__u32)saddr->s6_addr32[0]);
	sum += carry;

	sum += (__u32)saddr->s6_addr32[1];
	carry = (sum < (__u32)saddr->s6_addr32[1]);
	sum += carry;

	sum += (__u32)saddr->s6_addr32[2];
	carry = (sum < (__u32)saddr->s6_addr32[2]);
	sum += carry;

	sum += (__u32)saddr->s6_addr32[3];
	carry = (sum < (__u32)saddr->s6_addr32[3]);
	sum += carry;

	sum += (__u32)daddr->s6_addr32[0];
	carry = (sum < (__u32)daddr->s6_addr32[0]);
	sum += carry;

	sum += (__u32)daddr->s6_addr32[1];
	carry = (sum < (__u32)daddr->s6_addr32[1]);
	sum += carry;

	sum += (__u32)daddr->s6_addr32[2];
	carry = (sum < (__u32)daddr->s6_addr32[2]);
	sum += carry;

	sum += (__u32)daddr->s6_addr32[3];
	carry = (sum < (__u32)daddr->s6_addr32[3]);
	sum += carry;

	ulen = (__u32)htonl((__u32) len);
	sum += ulen;
	carry = (sum < ulen);
	sum += carry;

	uproto = (__u32)htonl(proto);
	sum += uproto;
	carry = (sum < uproto);
	sum += carry;

	return csum_fold((__wsum)sum);
}
#endif

uint16_t tcp_checksum(const void *buff, size_t len, struct in6_addr *src_addr, struct in6_addr *dest_addr)
{
    return from32to16(csum_ipv6_magic(src_addr, dest_addr,
                len, 6, (uint32_t) do_csum(buff, len )));
}

uint16_t set_tcp_checksum(struct parser_ipv6 *ip6hdr, struct parser_tcp *tcphdr, size_t len) {
    tcphdr->checksum = 0;
    tcphdr->checksum = tcp_checksum(tcphdr, len, &(ip6hdr->src), &(ip6hdr->dst));
    return tcphdr->checksum;
}

#endif
