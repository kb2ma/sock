#ifndef SOCK_UDP__
#define SOCK_UDP__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "net/ipv6/addr.h"

#include "sock_udp_linux.h"

#define IP4(a,b,c,d) (uint32_t)((a<<24) | b<<16 | c<<8 | a)

#define SOCK_UDP_LOCAL (0x1)
#define SOCK_UDP_REMOTE (0x2)

typedef struct udp_endpoint6 {
    uint16_t port;
    uint16_t iface;
    ipv6_addr_t *addr;
} udp_endpoint6_t;

typedef struct udp_endpoint4 {
    uint16_t port;
    uint16_t iface;
    ipv4_addr_t addr;
} udp_endpoint4_t;

typedef struct udp_endpoint {
    unsigned family;
    union {
        udp_endpoint6_t ipv6;
        udp_endpoint4_t ipv4;
    };
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
