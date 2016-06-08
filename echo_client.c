#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "sock_udp.h"

int main(int argc, char *argv[])
{
    char buf[128];

    if (argc > 1) {
        memcpy(buf, argv[1], strlen(argv[1]));
    }
    else {
        const char *tmp = "Hello World!";
        memcpy(buf, tmp, strlen(tmp)+1);
    }

    sock_udp_t sock;
    udp_endpoint_t local = { .family=AF_INET6 };
    udp_endpoint_t remote = { .family=AF_INET6, .port=60001 };

    remote.addr.ipv6.addr[15] = 0x1; /* ::1 */

    ssize_t res = sock_udp_init(&sock, &local, &remote);
    if (res == -1) {
        return -1;
    }

    res = sock_udp_send(&sock, buf, strlen(buf));
    if (res == -1) {
        perror("recv");
        return -1;
    }
    else {
        memset(buf, '\0', sizeof(buf));
        res = sock_udp_recv(&sock, buf, sizeof(buf), 10000U, NULL);
        printf("res=%zi text=\"%s\"\n", res, buf);
    }

    return 0;
}
