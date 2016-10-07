#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "nanocoap.h"
#include "net/sock/udp.h"

#include "net/sock/util.h"

ssize_t coap_get(const char *url, uint8_t *buf, size_t len)
{
    ssize_t res;
    sock_udp_t sock;
    sock_udp_ep_t local = { 0 };
    sock_udp_ep_t remote;

    char hostport[SOCK_HOSTPORT_MAXLEN] = {0};
    char urlpath[SOCK_URLPATH_MAXLEN] = {0};

    if (strncmp(url, "coap://", 7)) {
        return -EINVAL;
    }

    res = sock_urlsplit(url, hostport, urlpath);
    if (res) {
        return res;
    }

    res = sock_str2ep(&remote, hostport);
    if (res) {
        return res;
    }

    if (!remote.port) {
        remote.port = COAP_PORT;
    }

    res = sock_udp_create(&sock, &local, NULL, 0);
    if (res < 0) {
        return res;
    }

    uint8_t *pktpos = buf;
    pktpos += coap_build_hdr((coap_hdr_t *)pktpos, COAP_REQ, NULL, 0, COAP_METHOD_GET, 1);
    pktpos += coap_put_option_url(pktpos, 0, urlpath);

    /* TODO: timeout random between between ACK_TIMEOUT and (ACK_TIMEOUT *
     * ACK_RANDOM_FACTOR) */
    uint32_t timeout = COAP_ACK_TIMEOUT * (1000000U);
    int tries = 0;
    while (tries++ < COAP_MAX_RETRANSMIT) {
        if(!tries) {
            printf("nanocoap: maximum retries reached.\n");
            res = -ETIMEDOUT;
            goto out;
        }

        res = sock_udp_send(&sock, buf, pktpos-buf, &remote);
        if (res <= 0) {
            printf("nanocoap: error sending coap request\n");
            goto out;
        }
        res = sock_udp_recv(&sock, buf, len, timeout, NULL);
        if (res <= 0) {
            if (res == -ETIMEDOUT) {
                printf("nanocoap: timeout\n");

                timeout *= 2;
                continue;
            }
            printf("nanocoap: error sending coap request\n");
            goto out;
        }

        coap_pkt_t pkt;
        if (coap_parse(&pkt, (uint8_t*)buf, res) < 0) {
            puts("error parsing packet");
            continue;
        }
        printf("recv res=%zi\n", res);
        if (pkt.hdr->code == COAP_CODE_205) {
            printf("nanocoap payload length=%u content: \"%*s\"\n", pkt.payload_len, pkt.payload_len, pkt.payload);
            res = 0;
            goto out;
        }
    }

out:
    sock_udp_close(&sock);

    return res;
}
