#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "sock_udp.h"

int main(int argc, char *argv[])
{
    char buf[128];

    sock_udp_t sock;
    udp_endpoint_t local = { .family=AF_INET6, .ipv6.port=60001 };
    ipv6_addr_t remote_addr;
    udp_endpoint_t remote = { .ipv6.addr=&remote_addr };

    ssize_t res = sock_udp_init(&sock, &local, NULL);
    if (res == -1) {
        return -1;
    }

    while(1) {
        res = sock_udp_recv(&sock, buf, sizeof(buf), -1, &remote);
        if (res == -1) {
            perror("recv");
            return -1;
        }
        else {
            buf[res] = '\0';
            printf("res=%zu text=\"%s\"\n", res, buf);
            sock_udp_set_dst(&sock, &remote);
            sock_udp_send(&sock, buf, res);
        }
    }

    return 0;
}
