#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "net/sock/udp.h"

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
