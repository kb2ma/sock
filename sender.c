#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "sock_udp.h"

int main(int argc, char *argv[])
{
//    uint8_t dest[16];
//    inet_pton(AF_INET6, "fe80::1", dest);
//    unsigned ifindex = if_nametoindex("tap0");

//    udp_endpoint_t src = { .family=AF_INET6, .ipv6.port=60000, .ipv6.iface=ifindex };
//    udp_endpoint_t dst = { .family=AF_INET6, .ipv6.addr=(ipv6_addr_t*)dest, .ipv6.port=60001, .ipv6.iface=ifindex };
    udp_endpoint_t src = { .family=AF_INET, .ipv4.port=60000 };
    udp_endpoint_t dst = { .family=AF_INET, .ipv4.port=60001 };
    inet_pton(AF_INET, "127.0.0.1", &dst.ipv4.addr);
    inet_pton(AF_INET, "192.168.1.150", &src.ipv4.addr);

    sock_udp_t sock;

    ssize_t res = sock_udp_init(&sock, &src, &dst);
    if (res == -1) {
        return -1;
    }

    char buf[128];
    sprintf(buf, "TEST");

    while(1) {
        res = sock_udp_send(&sock, buf, strlen(buf));
        printf("res=%zi\n", res);
        if (res == -1) {
            return -1;
        }
        sleep(1);
    }

    return 0;
}
