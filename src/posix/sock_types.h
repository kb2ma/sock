#include <arpa/inet.h>

#if defined(SOCK_HAS_IPV6)
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
