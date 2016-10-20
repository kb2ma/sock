#ifndef SOCK_DNS_H
#define SOCK_DNS_H

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    uint8_t payload[];
} sock_dns_hdr_t;

#define DNS_TYPE_A              (1)
#define DNS_TYPE_AAAA           (28)
#define DNS_CLASS_IN            (1)

#define SOCK_DNS_PORT           (53)
#define SOCK_DNS_RETRIES        (2)

#define SOCK_DNS_MAX_NAME_LEN   (64U)       /* we're in embedded context. */
#define SOCK_DNS_QUERYBUF_LEN   (sizeof(sock_dns_hdr_t) + 4 + SOCK_DNS_MAX_NAME_LEN)

int sock_dns_query(const char *domain_name, void *addr_out, int family);

#endif /* SOCK_DNS_H */
