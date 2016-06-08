#ifndef SOCK_UDP__
#define SOCK_UDP__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>

#if ! (defined(SOCK_UDP_IPV4) || defined(SOCK_UDP_IPV6))
#define SOCK_UDP_IPV4
#define SOCK_UDP_IPV6
#endif

#include "net/ipv6/addr.h"
#include "sock_udp_linux.h"

#define IP4(a,b,c,d) (uint32_t)((a<<24) | b<<16 | c<<8 | a)

#define SOCK_UDP_LOCAL (0x1)
#define SOCK_UDP_REMOTE (0x2)

#define SOCK_NO_TIMEOUT (0x0-1)

typedef struct udp_endpoint {
    uint16_t family;
    uint16_t port;
    uint16_t iface;
    union {
#if defined(SOCK_UDP_IPV6)
        ipv6_addr_t ipv6;
#endif
#if defined(SOCK_UDP_IPV4)
        ipv4_addr_t ipv4;
#endif
    } addr;
} udp_endpoint_t;

typedef struct sock_udp sock_udp_t;

ssize_t sock_udp_sendto(const udp_endpoint_t *dst, const void* data, size_t len, uint16_t src_port);
int sock_udp_init(sock_udp_t *sock, const udp_endpoint_t *local, const udp_endpoint_t *remote);
int sock_udp_set_dst(sock_udp_t *sock, const udp_endpoint_t *dst);
ssize_t sock_udp_send(sock_udp_t *sock, const void* data, size_t len);
ssize_t sock_udp_recv(sock_udp_t *sock, void* buf, size_t len, unsigned timeout, udp_endpoint_t *remote);
void sock_udp_close(sock_udp_t *sock);
int sock_udp_fmt_endpoint(const udp_endpoint_t *endpoint, char *addr_str, uint16_t *port);

#endif /* SOCK_UDP__ */
