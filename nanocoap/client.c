#include <assert.h>
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
        fprintf(stderr, "usage: %s <url>\n", argv[0]);
        return 1;
    }

    ssize_t res = coap_get(argv[1], buf, sizeof(buf));
    if (res <= 0) {
        fprintf(stderr, "error %zi\n", res);
        return 1;
    }
    else {
        assert((unsigned)res < sizeof(buf));
        printf("%.*s\n", (int)res, buf);
        return 0;
    }
}

