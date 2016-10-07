#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "net/sock/udp.h"
#include "nanocoap.h"

#include "net/sock/util.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    uint8_t buf[128];

    sock_udp_t sock;
    sock_udp_ep_t local = { .port=COAP_PORT };
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
            char addr_str[INET6_ADDRSTRLEN+5];
            memset(addr_str, '\0', sizeof(addr_str));
            uint16_t port;
            sock_udp_fmt_endpoint(&remote, addr_str, &port);
            buf[res] = '\0';
            printf("recv %zi from %s port %u.\n", res, addr_str, remote.port);
            coap_pkt_t pkt;
            if (coap_parse(&pkt, (uint8_t*)buf, res) < 0) {
                puts("error parsing packet");
                continue;
            }
            if ((res = coap_handle_req(&pkt, buf, sizeof(buf))) > 0) {
                res = sock_udp_send(&sock, buf, res, &remote);
                printf("%zi\n", res);
            }
        }
    }

    return 0;
}
