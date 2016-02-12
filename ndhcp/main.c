#include <stdlib.h>
#include <net/if.h>
#include <stdio.h>
#include <unistd.h>

#include "sock_udp.h"
#include "ndhcpc.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <iface>\n", argv[0]);
        exit(1);
    }

    unsigned ifindex = if_nametoindex(argv[1]);

    printf("%u\n", ifindex);

    ndhcpc_t ndhcpc;
    ndhcpc_init(&ndhcpc, ifindex, 0x12345678);
    ndhcpc_work(&ndhcpc);

    return 0;
}
