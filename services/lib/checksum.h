
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "../../seg6ctl/libnlmem/nlmem.h"
#include "../../seg6ctl/libseg6/seg6.h"
#include <unistd.h>

#include "parsing.h"

uint16_t set_tcp_checksum(struct parser_ipv6 *ip6hdr, struct parser_tcp *tcphdr, size_t len);

inline __sum16 csum_fold(__wsum csum);
__sum16 csum_fold(__wsum csum);

unsigned int csum_partial(const void *buff, int len, unsigned int wsum);

inline unsigned short from32to16(unsigned int x);
unsigned short from32to16(unsigned int x);

unsigned int do_csum(const unsigned char *buff, int len);


__sum16 csum_ipv6_magic(const struct in6_addr *saddr,
			const struct in6_addr *daddr,
			__u32 len, unsigned short proto,
			__wsum csum);

uint16_t tcp_checksum(const void *buff, size_t len, struct in6_addr *src_addr, struct in6_addr *dest_addr);

uint16_t set_tcp_checksum(struct parser_ipv6 *ip6hdr, struct parser_tcp *tcphdr, size_t len);
