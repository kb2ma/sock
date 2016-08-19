#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "net/sock/udp.h"

int main(int argc, char *argv[])
{
    char buf[128];

    sock_udp_t sock;
    sock_udp_ep_t local = { .family=AF_INET6, .port=60001 };
    sock_udp_ep_t remote = { 0 };

    ssize_t res = sock_udp_create(&sock, &local, NULL, 0);
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
            sock_udp_send(&sock, buf, res, &remote);
        }
    }

    return 0;
}
