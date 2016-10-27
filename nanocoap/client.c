#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "net/sock/udp.h"
#include "net/sock/util.h"

#include "nanocoap.h"
#include "nanocoap_sock.h"

int main(int argc, char *argv[])
{
    uint8_t buf[128];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <url>\n", argv[0]);
        return 1;
    }

    char *url = argv[1];
    sock_udp_ep_t remote;

    char hostport[SOCK_HOSTPORT_MAXLEN] = {0};
    char urlpath[SOCK_URLPATH_MAXLEN] = {0};

    if (strncmp(url, "coap://", 7)) {
        return 1;
    }

    ssize_t res = sock_urlsplit(url, hostport, urlpath);
    if (res) {
        return 1;
    }

    res = sock_str2ep(&remote, hostport);
    if (res) {
        return 1;
    }

    res = nanocoap_get(&remote, urlpath, buf, sizeof(buf));
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

