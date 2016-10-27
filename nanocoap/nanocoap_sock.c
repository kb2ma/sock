#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "nanocoap.h"
#include "net/sock/udp.h"

#include "net/sock/util.h"

#if NANOCOAP_DEBUG
#define ENABLE_DEBUG (1)
#else
#define ENABLE_DEBUG (0)
#endif
#include "debug.h"

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
            printf("nanocoap: error receiving coap request\n");
            break;
        }

        coap_pkt_t pkt;
        if (coap_parse(&pkt, (uint8_t*)buf, res) < 0) {
            puts("error parsing packet");
            continue;
        }
        else {
            res = coap_get_code(&pkt);
            if (res != 205) {
                res = -res;
            }
            else {
                if (pkt.payload_len) {
                    memcpy(buf, pkt.payload, pkt.payload_len);
                }
                res = pkt.payload_len;
            }
            break;
        }
    }

out:
    sock_udp_close(&sock);

    return res;
}

int nanocoap_server(sock_udp_ep_t *local, uint8_t *buf, size_t bufsize)
{
    sock_udp_t sock;
    sock_udp_ep_t remote = { 0 };

    if (!local->port) {
        local->port = COAP_PORT;
    }

    ssize_t res = sock_udp_create(&sock, local, NULL, 0);
    if (res == -1) {
        return -1;
    }

    while(1) {
        res = sock_udp_recv(&sock, buf, bufsize, -1, &remote);
        if (res == -1) {
            perror("recv");
            return -1;
        }
        else {
            coap_pkt_t pkt;
            if (coap_parse(&pkt, (uint8_t*)buf, res) < 0) {
                DEBUG("error parsing packet\n");
                continue;
            }
            if ((res = coap_handle_req(&pkt, buf, bufsize)) > 0) {
                res = sock_udp_send(&sock, buf, res, &remote);
            }
        }
    }

    return 0;
}
