#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "net/sock/udp.h"

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
    sock_udp_ep_t local = { .family=AF_INET6 };
    sock_udp_ep_t remote = { .family=AF_INET6, .port=60001 };

    remote.addr.ipv6.u8[15] = 0x1; /* ::1 */

    ssize_t res = sock_udp_create(&sock, &local, &remote, 0);
    if (res == -1) {
        return -1;
    }

    res = sock_udp_send(&sock, buf, strlen(buf), NULL);
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
