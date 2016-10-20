#include <stddef.h>
#include <stdio.h>



#include "sock_dns.h"

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
