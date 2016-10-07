#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>

#include "net/sock/udp.h"
#include "net/sock/util.h"

int sock_udp_fmt_endpoint(const sock_udp_ep_t *endpoint, char *addr_str, uint16_t *port)
{
    void *addr_ptr;
    *addr_str = '\0';

    if (endpoint->family==AF_INET) {
#if defined(SOCK_HAS_IPV4)
        addr_ptr = (void*)&endpoint->addr.ipv4;
#else
        return -ENOTSUP;
#endif
    }
    else {
#if defined(SOCK_HAS_IPV6)
        addr_ptr = (void*)&endpoint->addr.ipv6;
#else
        return -ENOTSUP;
#endif
    }

    if (!inet_ntop(endpoint->family, addr_ptr, addr_str, INET6_ADDRSTRLEN)) {
        return 0;
    }

#if defined(SOCK_HAS_IPV6)
    if ((endpoint->family == AF_INET6) && endpoint->netif) {
        sprintf(addr_str + strlen(addr_str), "%%%4u", endpoint->netif);
    }
#endif

    if (port) {
        *port = endpoint->port;
    }

    return strlen(addr_str);
}

static char* _find_hoststart(const char *url)
{
    char *urlpos = (char*)url;
    while(*urlpos) {
        if (*urlpos++ == ':') {
            if (strncmp(urlpos, "//", 2) == 0) {
                return urlpos + 2;
            }
            break;
        }
        urlpos++;
    }
    return NULL;
}

static char* _find_pathstart(const char *url)
{
    char *urlpos = (char*)url;
    while(*urlpos) {
        if (*urlpos == '/') {
            return urlpos;
        }
        urlpos++;
    }
    return NULL;
}

int sock_urlsplit(const char *url, char *hostport, char *urlpath)
{
    char *hoststart = _find_hoststart(url);
    if (!hoststart) {
        return -EINVAL;
    }

    char *pathstart = _find_pathstart(hoststart);
    if(!pathstart) {
        return -EINVAL;
    }

    memcpy(hostport, hoststart, pathstart - hoststart);

    size_t pathlen = strlen(pathstart);
    if (pathlen) {
        memcpy(urlpath, pathstart, pathlen);
    }
    else {
        *urlpath = '\0';
    }
    return 0;
}

int sock_str2ep(sock_udp_ep_t *ep_out, const char *str)
{
    unsigned brackets_flag;
    char *hoststart = (char*)str;
    char *hostend = hoststart;

    char hostbuf[64];

    memset(ep_out, 0, sizeof(sock_udp_ep_t));

    if (*hoststart == '[') {
        brackets_flag = 1;
        for (hostend = ++hoststart; *hostend && *hostend != ']';
                hostend++);
        if (! *hostend) {
            /* none found, bail out */
            return -EINVAL;
        }
    }
    else {
        brackets_flag = 0;
        for (hostend = hoststart; *hostend && *hostend != ':';
                hostend++);
    }

    if (*(hostend + brackets_flag) == ':') {
        ep_out->port = atoi(hostend + brackets_flag + 1);
    }

    size_t hostlen = hostend - hoststart;
    if (hostlen >= sizeof(hostbuf)) {
        return -EINVAL;
    }

    memcpy(hostbuf, hoststart, hostlen);

    hostbuf[hostlen] = '\0';

    if (!brackets_flag) {
        if (inet_pton(AF_INET, hostbuf, &ep_out->addr.ipv4) == 1) {
            ep_out->family = AF_INET;
            return 0;
        }
    }
#if defined(SOCK_HAS_IPV6)
    if (inet_pton(AF_INET6, hostbuf, ep_out->addr.ipv6) == 1) {
        ep_out->family = AF_INET6;
        return 0;
    }
#endif
    return -EINVAL;
}
