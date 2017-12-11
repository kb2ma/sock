#include <stdint.h>

#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "net/sock/udp.h"

#define COAP_INBUF_SIZE (256U)

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    uint8_t buf[COAP_INBUF_SIZE];

    sock_udp_ep_t local = { .port=COAP_PORT };

    nanocoap_server(&local, buf, sizeof(buf));

    return 0;
}
