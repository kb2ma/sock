#include <stddef.h>
#include <stdio.h>

#include "sock_dns.h"

sock_udp_ep_t sock_dns_server = { .family=AF_INET, .port=SOCK_DNS_PORT,
                                  .addr.ipv4={8,8,8,8}
                                };

int main(int argc, char *argv[]) {
    uint8_t addr[16] = {0};
    char addrstr[INET6_ADDRSTRLEN];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    int res = sock_dns_query(argv[1], addr, AF_UNSPEC);
    if (res > 0) {
        inet_ntop(res == 4 ? AF_INET : AF_INET6, addr, addrstr, sizeof(addrstr));
        printf("%s\n", addrstr);
    }

    return 0;
}
