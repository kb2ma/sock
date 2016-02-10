#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#include "sock_udp.h"

int ipv6_addr_is_multicast(ipv6_addr_t* addr)
{
    return addr->addr[0] == 0xFF;
}

int ipv6_addr_is_unspecified(ipv6_addr_t* addr)
{
    return memcmp(addr, &in6addr_any, 16) == 0;
}

static int _addrlen(unsigned family) {
    return (family == AF_INET6) ?
        sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
}

static int _endpoint_to_sockaddr(struct sockaddr *sockaddr, const udp_endpoint_t *endpoint)
{
    assert(sockaddr);
    assert(endpoint);

    switch(endpoint->family) {
        case AF_INET6:
            {
                struct sockaddr_in6 *dst_addr6 = (struct sockaddr_in6 *) sockaddr;
                dst_addr6->sin6_family = AF_INET6;
                if (endpoint->ipv6.addr) {
                    memcpy(&dst_addr6->sin6_addr, endpoint->ipv6.addr, 16);
                }
                else {
                    dst_addr6->sin6_addr = in6addr_any;
                }
                dst_addr6->sin6_port = htons(endpoint->ipv6.port);
                dst_addr6->sin6_scope_id = endpoint->ipv6.iface;
                return 0;
            }
        default:
            return -EINVAL;
    }
}

ssize_t sock_udp_sendto(const udp_endpoint_t *dst, const void* data, unsigned len, uint16_t src_port)
{
    ssize_t res;
    struct sockaddr dst_addr = {0};
    memset((void*)&dst_addr, '\0', sizeof(dst_addr));
    if (_endpoint_to_sockaddr(&dst_addr, dst)) {
        return -EINVAL;
    }

    int fd = socket(dst->family, SOCK_DGRAM, 0);
    if (fd == -1) {
        return -1;
    }

    if ((dst->family == AF_INET6) && ipv6_addr_is_multicast(dst->ipv6.addr)) {
        unsigned ifindex = dst->ipv6.iface;
        setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex,
                   sizeof(ifindex));
    }

    if (src_port) {
        struct sockaddr src_addr;
        unsigned addr_len;
        memset((void*)&src_addr, '\0', sizeof(src_addr));
        switch (dst->family) {
            case AF_INET6:
                {
                    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &src_addr;
                    sin6->sin6_family = AF_INET6;
                    sin6->sin6_port = htons(src_port);
                    sin6->sin6_addr = in6addr_any;
                    addr_len = sizeof(struct sockaddr_in6);
                    break;
                }
            default:
                return -EINVAL;
        }

        const int on=1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(fd, &src_addr, addr_len) == -1) {
            res = -1;
            goto close;
        }
    }

    size_t dst_addr_len = (dst->family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);

    if ((res = sendto(fd, data, len, 0, &dst_addr, dst_addr_len)) == -1) {
        res = -1;
    }

close:
    close(fd);

    return res;
}

int sock_udp_init(sock_udp_t *sock, const udp_endpoint_t *src, const udp_endpoint_t *dst)
{
    int res;
    struct sockaddr src_addr;

    if (!(src || dst)) {
        return -EINVAL;
    }

    if ((src && dst) && (src->family != dst->family)) {
        return -EINVAL;
    }

    unsigned family = AF_INET6;

    if (src) {
        family = src->family;

        memset((void*)&src_addr, '\0', sizeof(src_addr));
        _endpoint_to_sockaddr(&src_addr, src);
    }

    if (dst) {
        if (!src) {
            family = dst->family;
        }
        /* TODO: if set, it can't be unspecified */
        memset((void*)&sock->peer, '\0', sizeof(struct sockaddr));
        _endpoint_to_sockaddr(&sock->peer, dst);
    }

    int fd = socket(family, SOCK_DGRAM, 0);
    if (fd == -1) {
        return -1;
    }

    unsigned addr_len = _addrlen(family);

    if (bind(fd, &src_addr, addr_len) == -1) {
        res = -1;
        perror("bind");
        goto close;
    }

    sock->fd = fd;
    sock->flags = (dst != NULL);
    sock->family = family;
    return 0;

close:
    close(fd);

    return res;
}

void sock_udp_close(sock_udp_t *sock)
{
    if (sock && sock->fd) {
        close(sock->fd);
    }
}

int sock_udp_set_dst(sock_udp_t *sock, const udp_endpoint_t *dst)
{
    assert(sock);

    if (!dst) {
        sock->flags = 0;
        return 0;
    }

    if (sock->family != dst->family) {
        return -EINVAL;
    }

  /*  if (ipv6_addr_is_unspecified(dst->ipv6.addr)) {
        return -EINVAL;
    }*/

    memset((void*)&sock->peer, '\0', sizeof(struct sockaddr));
    _endpoint_to_sockaddr(&sock->peer, dst);

    sock->flags = 1;

    return 0;
}

ssize_t sock_udp_send(sock_udp_t *sock, const void* data, unsigned len)
{
    assert(sock);

    /* can only send on sockets with target address */
    if (!sock->flags) {
        return -EINVAL;
    }

    int res = sendto(sock->fd, data, len, 0, &sock->peer, _addrlen(sock->family));

    return res;
}
