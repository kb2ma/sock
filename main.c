#include <stdio.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "sock_udp.h"

int main(int argc, char *argv[])
{
    /*
    uint8_t dest[16];
    inet_pton(AF_INET6, "ff15::aaaa", dest);

    unsigned ifindex = if_nametoindex("tap0");
    udp_endpoint_t dst = { .family=AF_INET6, .ipv6.addr=(ipv6_addr_t*)dest, .ipv6.port=12345, .ipv6.iface=ifindex };

    char buf[] = "TEST";
    ssize_t res = sock_udp_sendto(&dst, buf, sizeof(buf), 60000);
    printf("res=%zi\n", res);
    perror("send");
*/
    char dest[16];
    inet_pton(AF_INET6, "fe80::2", dest);

    udp_endpoint_t src = { .family=AF_INET6, .ipv6.port=60000 };
    sock_udp_t sock;

    ssize_t res = sock_udp_init(&sock, &src, NULL);
    if (res == -1) {
        return -1;
    }

    char buf[] = "TEST";

    unsigned ifindex = if_nametoindex("tap0");

    udp_endpoint_t dst = { .family=AF_INET6, .ipv6.addr=(ipv6_addr_t*)dest, .ipv6.port=60000, .ipv6.iface=ifindex };

    printf("%p\n", &src);
    res = sock_udp_send(&sock, buf, sizeof(buf));
    printf("send res=%zi\n", res);

    sock_udp_set_dst(&sock, &dst);

    res = sock_udp_send(&sock, buf, sizeof(buf));
    printf("send2 res=%zi\n", res);

    sock_udp_close(&sock);

    return 0;
}
