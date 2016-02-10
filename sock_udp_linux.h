struct sock_udp {
    int fd;
    unsigned flags;
    unsigned family;
    struct sockaddr_in6 peer;
};

