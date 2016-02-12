#define _DEFAULT_SOURCE

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include <stdio.h>

#include "sock_udp.h"

static int _bind_to_device(int fd, unsigned iface);

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

static int _endpoint_to_sockaddr(void *sockaddr, const udp_endpoint_t *endpoint)
{
    assert(sockaddr);
    assert(endpoint);

    switch(endpoint->family) {
        case AF_INET:
            {
                struct sockaddr_in *dst_addr = sockaddr;
                dst_addr->sin_family = AF_INET;
                dst_addr->sin_port = htons(endpoint->ipv4.port);
                if (endpoint->ipv4.addr) {
                    dst_addr->sin_addr.s_addr = endpoint->ipv4.addr;
                }
                else {
                    puts("any");
                    dst_addr->sin_addr.s_addr = INADDR_ANY;
                }
                return 0;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *dst_addr6 = /*(struct sockaddr_in6 *)*/ sockaddr;
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

static int _sockaddr_to_endpoint(udp_endpoint_t *endpoint, void *_sockaddr)
{
    assert(_sockaddr);
    assert(endpoint);

    struct sockaddr *sockaddr = _sockaddr;
    switch(sockaddr->sa_family) {
        case AF_INET:
            {
                struct sockaddr_in *addr = (struct sockaddr_in*) sockaddr;
                endpoint->family = AF_INET;
                endpoint->ipv4.port = ntohs(addr->sin_port);
                endpoint->ipv4.addr = addr->sin_addr.s_addr;
                return 0;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *addr = (struct sockaddr_in6*) sockaddr;
                endpoint->family = AF_INET6;
                endpoint->ipv6.port = ntohs(addr->sin6_port);
                endpoint->ipv6.iface = addr->sin6_scope_id;
                memcpy(endpoint->ipv6.addr, &addr->sin6_addr, 16);
                return 0;
            }
        default:
            return -EINVAL;
    }
}

ssize_t sock_udp_sendto(const udp_endpoint_t *dst, const void* data, size_t len, uint16_t src_port)
{
    ssize_t res;
    struct sockaddr_in6 dst_addr = {0};
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

    if ((dst->family == AF_INET) && (dst->ipv4.addr == INADDR_BROADCAST)) {
        int enable = 1;
        setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
    }

    if (src_port) {
        struct sockaddr_in6 src_addr;
        unsigned addr_len;
        memset((void*)&src_addr, '\0', sizeof(src_addr));
        switch (dst->family) {
            case AF_INET:
                {
                    struct sockaddr_in *src_addr4 = (struct sockaddr_in *) &src_addr;
                    src_addr4->sin_family = AF_INET;
                    src_addr4->sin_port = htons(src_port);
                    src_addr4->sin_addr.s_addr = INADDR_ANY;
                    addr_len = sizeof(struct sockaddr_in);
                    break;
                }
            case AF_INET6:
                {
                    src_addr.sin6_family = AF_INET6;
                    src_addr.sin6_port = htons(src_port);
                    src_addr.sin6_addr = in6addr_any;
                    addr_len = sizeof(struct sockaddr_in6);
                    break;
                }
            default:
                return -EINVAL;
        }

        const int on=1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(fd, (struct sockaddr *)&src_addr, addr_len) == -1) {
            res = -1;
            goto close;
        }
    }

    if ((dst->family == AF_INET) && dst->ipv4.iface) {
        _bind_to_device(fd, dst->ipv4.iface);
    }

    size_t dst_addr_len = _addrlen(dst->family);

    if ((res = sendto(fd, data, len, 0, (struct sockaddr *)&dst_addr, dst_addr_len)) == -1) {
        res = -1;
    }

close:
    close(fd);

    return res;
}

int sock_udp_init(sock_udp_t *sock, const udp_endpoint_t *src, const udp_endpoint_t *dst)
{
    int res;
    struct sockaddr_in6 src_addr;

    if (!(src || dst)) {
        return -EINVAL;
    }

    if ((src && dst) && (src->family != dst->family)) {
        return -EINVAL;
    }

    if (src) {
        sock->family = src->family;
    }
    else {
        sock->family = dst->family;
    }

    sock->fd = socket(sock->family, SOCK_DGRAM, IPPROTO_IP);
    if (sock->fd == -1) {
        perror("creating socket");
        return -1;
    }

    if (src) {
        sock->family = src->family;

        memset((void*)&src_addr, '\0', sizeof(src_addr));
        _endpoint_to_sockaddr(&src_addr, src);

        unsigned addr_len = _addrlen(sock->family);

        const int on=1;
        setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        setsockopt(sock->fd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on));

        if (bind(sock->fd, (struct sockaddr *)&src_addr, addr_len) == -1) {
            res = -1;
            perror("bind");
            goto close;
        }
        sock->flags |= SOCK_UDP_LOCAL;
    }

    if (dst) {
        if (!src) {
            sock->family = dst->family;
        }
        if ((res = sock_udp_set_dst(sock, dst))) {
            goto close;
        }
    }

    return 0;

close:
    close(sock->fd);

    return res;
}

void sock_udp_close(sock_udp_t *sock)
{
    if (sock && sock->fd) {
        close(sock->fd);
    }
}

static int _bind_to_device(int fd, unsigned iface)
{
    struct ifreq ifr;
    memset(&ifr, '\0', sizeof(ifr));

    if (iface) {
        ifr.ifr_ifindex = iface;
        if (ioctl(fd, SIOCGIFNAME, &ifr)==-1) {
            puts("error getting ifname");
            return -1;
        }
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                (void *)&ifr, sizeof(ifr)) < 0) {
        perror("error setting SO_BINDTODEVICE");
        return -1;
    }

    return 0;
}

int sock_udp_set_dst(sock_udp_t *sock, const udp_endpoint_t *dst)
{
    assert(sock);

    if (!dst) {
        puts("a");
        sock->flags &= ~SOCK_UDP_REMOTE;
        return 0;
    }

    if (sock->family != dst->family) {
        puts("b");
        return -EINVAL;
    }

  /*  if (ipv6_addr_is_unspecified(dst->ipv6.addr)) {
        return -EINVAL;
    }*/

    if (dst->family == AF_INET) {
        if  (dst->ipv4.addr == 0xFFFFFFFF) {
            int enable = 1;
            if (setsockopt(sock->fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) == -1) {
                perror("enabling broadcast");
            }
        }

        /* if an interface is specified, force it's usage.
         * if dst->iface is 0, this will unbind. */
        _bind_to_device(sock->fd, dst->ipv4.iface);
    }

    memset((void*)&sock->peer, '\0', sizeof(struct sockaddr_in6));
    _endpoint_to_sockaddr(&sock->peer, dst);

    sock->flags |= SOCK_UDP_REMOTE;

    return 0;
}

ssize_t sock_udp_send(sock_udp_t *sock, const void* data, size_t len)
{
    assert(sock);

    /* can only send on sockets with target address */
    if (!(sock->flags & SOCK_UDP_REMOTE)) {
        puts("c");
        return -EINVAL;
    }

    int res = sendto(sock->fd, data, len, 0, (struct sockaddr *)&sock->peer, _addrlen(sock->family));
    if (res==-1) {
        perror("sendto");
    }

    return res;
}

ssize_t sock_udp_recv(sock_udp_t *sock, void* buf, size_t len, unsigned timeout, udp_endpoint_t *remote)
{
    /* can only receive from sockets bound to address (or in(6)addr_any) */
    if (!(sock->flags & SOCK_UDP_LOCAL)) {
        return -EINVAL;
    }

    fd_set _select_fds;
    if (timeout != ((unsigned)-1)) {
        int activity;
        struct timeval _timeout = { .tv_usec=timeout };

        FD_ZERO(&_select_fds);
        FD_SET(sock->fd, &_select_fds);
        activity = select(sock->fd + 1, &_select_fds, NULL, NULL, &_timeout);

        if (activity <= 0) {
            return -ETIMEDOUT;
        }
    }

    struct sockaddr_in6 sockaddr_remote = {0};
    unsigned socklen;
    ssize_t res = recvfrom(sock->fd, buf, len, 0, (struct sockaddr *)&sockaddr_remote, &socklen);
    if (remote && (res != -1)) {
        _sockaddr_to_endpoint(remote, &sockaddr_remote);
    }
    return res;
}

int sock_udp_fmt_endpoint(const udp_endpoint_t *endpoint, char *addr_str, uint16_t *port)
{
    void *addr_ptr;
    *addr_str = '\0';

    if (endpoint->family==AF_INET) {
        addr_ptr = (void*)&endpoint->ipv4.addr;
    }
    else {
        addr_ptr = endpoint->ipv6.addr;
    }

    if (!inet_ntop(endpoint->family, addr_ptr, addr_str, INET6_ADDRSTRLEN)) {
        return 0;
    }

    if ((endpoint->family == AF_INET6) && endpoint->ipv6.iface) {
        sprintf(addr_str + strlen(addr_str), "%%%4u", endpoint->ipv6.iface);
    }

    if (port) {
        *port = endpoint->ipv4.port;
    }

    return strlen(addr_str);
}