#ifndef SOCK_UDP__
#define SOCK_UDP__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "net/ipv6/addr.h"

#include "sock_udp_linux.h"

typedef struct udp_endpoint6 {
    ipv6_addr_t *addr;
    uint16_t port;
    uint16_t iface;
} udp_endpoint6_t;

typedef struct udp_endpoint4 {
    uint16_t port;
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

ssize_t sock_udp_sendto(const udp_endpoint_t *dst, const void* data, unsigned len, uint16_t src_port);
int sock_udp_init(sock_udp_t *sock, const udp_endpoint_t *src, const udp_endpoint_t *dst);
int sock_udp_set_dst(sock_udp_t *sock, const udp_endpoint_t *dst);
ssize_t sock_udp_send(sock_udp_t *sock, const void* data, unsigned len);
ssize_t sock_udp_recv(sock_udp_t *sock, const void* data, unsigned len, unsigned timeout, udp_endpoint_t *src);
void sock_udp_close(sock_udp_t *sock);

#endif /* SOCK_UDP__ */
