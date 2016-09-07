#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "net/sock/udp.h"
#include "nanocoap.h"

int main(int argc, char *argv[])
{
    uint8_t buf[128];
    if (argc < 2) {
        printf("usage: %s <url>\n", argv[0]);
        return 1;
    }
    coap_get(argv[1], buf, sizeof(buf));
}

