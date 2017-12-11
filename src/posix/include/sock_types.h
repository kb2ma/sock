#include <arpa/inet.h>

#ifndef SOCK_HAS_IPV6
#define SOCK_HAS_IPV6   0
#endif

#ifndef SOCK_HAS_IPV4
#define SOCK_HAS_IPV4   0
#endif

#if SOCK_HAS_IPV6
    typedef struct sockaddr_in6 sockaddr_t;
#else
    typedef struct sockaddr_in sockaddr_t;
#endif

struct sock_udp {
    int fd;
    unsigned flags;
    int family;
    sockaddr_t peer;
};
