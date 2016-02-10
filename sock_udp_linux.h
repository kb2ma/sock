struct sock_udp {
    int fd;
    unsigned flags;
    unsigned family;
    struct sockaddr peer;
};

