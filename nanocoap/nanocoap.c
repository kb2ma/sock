#include <arpa/inet.h>
#include "nanocoap.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

static int _decode_value(unsigned val, uint8_t **pkt_pos_ptr, uint8_t *pkt_end);

ssize_t _test_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len)
{
    printf("_test_handler()\n");
    printf("coap pkt parsed. code=%u detail=%u payload_len=%u, 0x%02x\n",
            coap_get_code_class(pkt),
            coap_get_code_detail(pkt),
            pkt->payload_len, pkt->hdr->code);
    switch (coap_get_code_detail(pkt)) {
        case COAP_METHOD_GET:
            puts("get");
            break;
        default:
            puts("unhandled method");
            return -1;
    }

    return 0;
}

const coap_endpoint_t endpoints[] = {
    { "/test", COAP_METHOD_GET, _test_handler },
};

const unsigned endpoints_numof = sizeof(endpoints) / sizeof(endpoints[0]);

/* http://tools.ietf.org/html/rfc7252#section-3
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Ver| T |  TKL  |      Code     |          Message ID           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Token (if any, TKL bytes) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Options (if any) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |1 1 1 1 1 1 1 1|    Payload (if any) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
int coap_parse(coap_pkt_t *pkt, uint8_t *buf, size_t len)
{
    uint8_t *urlpos = pkt->url;
    coap_hdr_t *hdr = (coap_hdr_t *)buf;
    pkt->hdr = hdr;

    uint8_t *pkt_pos = hdr->data;
    uint8_t *pkt_end = buf + len;

    memset(pkt->url, '\0', NANOCOAP_URL_MAX);

    /* token value (tkl bytes) */
    pkt_pos += coap_get_token_len(pkt);

    /* parse options */
    int option_nr = 0;
    while (pkt_pos != pkt_end) {
        uint8_t option_byte = *pkt_pos++;
        if (option_byte == 0xff) {
            pkt->payload = pkt_pos;
            pkt->payload_len = buf + len - pkt_pos ;
            printf("payload len = %u\n", pkt->payload_len);
            break;
        }
        else {
            int option_delta = _decode_value(option_byte >> 4, &pkt_pos, pkt_end);
            if (option_delta < 0) {
                puts("bad op delta");
                return -EBADMSG;
            }
            int option_len = _decode_value(option_byte & 0xf, &pkt_pos, pkt_end);
            if (option_len < 0) {
                puts("bad op len");
                return -EBADMSG;
            }
            option_nr += option_delta;
            printf("option nr=%i len=%i\n", option_nr, option_len);

            switch (option_nr) {
                case COAP_OPT_URL:
                    {
                        *urlpos++ = '/';
                        memcpy(urlpos, pkt_pos, option_len);
                        urlpos += option_len;
                        break;
                    }
                default:
                    printf("nanocoap: unhandled option nr=%i len=%i\n", option_nr, option_len);
            }

            pkt_pos += option_len;
        }
    }

    printf("coap pkt parsed. code=%u detail=%u payload_len=%u, 0x%02x\n",
            coap_get_code_class(pkt),
            coap_get_code_detail(pkt),
            pkt->payload_len, hdr->code);

    return 0;
}

ssize_t coap_handle_req(coap_pkt_t *pkt, uint8_t *resp_buf, unsigned resp_buf_len)
{
    if (coap_get_code_class(pkt) != COAP_REQ) {
        puts("coap_handle_req(): not a request.");
        return -EBADMSG;
    }

    unsigned url_len = strlen((char*)pkt->url);
    unsigned endpoint_len;

    for (int i = 0; i < endpoints_numof; i++) {
        int res = strcmp((char*)pkt->url, endpoints[i].path);
        if (res < 0) {
            continue;
        }
        else if (res > 0) {
            break;
        }
        else {
            return endpoints[i].handler(pkt, resp_buf, resp_buf_len);
        }
    }
    printf("no handler found.\n");
}

static int _decode_value(unsigned val, uint8_t **pkt_pos_ptr, uint8_t *pkt_end)
{
    uint8_t *pkt_pos = *pkt_pos_ptr;
    size_t left = pkt_end - pkt_pos;
    int res;
    switch (val) {
        case 13:
            {
            /* An 8-bit unsigned integer follows the initial byte and
               indicates the Option Delta minus 13. */
            if (left < 1) {
                return -ENOSPC;
            }
            uint8_t delta = *pkt_pos++;
            res = delta + 13;
            break;
            }
        case 14:
            {
            /* A 16-bit unsigned integer in network byte order follows
             * the initial byte and indicates the Option Delta minus
             * 269. */
            if (left < 2) {
                return -ENOSPC;
            }
            uint16_t delta;
            uint8_t *_tmp = (uint8_t*)&delta;
            *_tmp++= *pkt_pos++;
            *_tmp++= *pkt_pos++;
            res = ntohs(delta) + 269;
            break;
            }
        case 15:
            /* Reserved for the Payload Marker.  If the field is set to
             * this value but the entire byte is not the payload
             * marker, this MUST be processed as a message format
             * error. */
            return -EBADMSG;
        default:
            res = val;
    }

    *pkt_pos_ptr = pkt_pos;
    return res;
}
