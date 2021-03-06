#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "nanocoap.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int _decode_value(unsigned val, uint8_t **pkt_pos_ptr, uint8_t *pkt_end);

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
            pkt->payload_len = buf + len - pkt_pos;
            DEBUG("payload len = %u\n", pkt->payload_len);
            break;
        }
        else {
            int option_delta = _decode_value(option_byte >> 4, &pkt_pos, pkt_end);
            if (option_delta < 0) {
                DEBUG("bad op delta\n");
                return -EBADMSG;
            }
            int option_len = _decode_value(option_byte & 0xf, &pkt_pos, pkt_end);
            if (option_len < 0) {
                DEBUG("bad op len\n");
                return -EBADMSG;
            }
            option_nr += option_delta;
            DEBUG("option nr=%i len=%i\n", option_nr, option_len);

            switch (option_nr) {
                case COAP_OPT_URL:
                    *urlpos++ = '/';
                    memcpy(urlpos, pkt_pos, option_len);
                    urlpos += option_len;
                    break;
                case COAP_OPT_CT:
                    if (option_len == 1) {
                        pkt->content_type = *pkt_pos;
                    } else {
                        memcpy(&pkt->content_type, pkt_pos, 2);
                        pkt->content_type = ntohs(pkt->content_type);
                    }
                    break;
                default:
                    DEBUG("nanocoap: unhandled option nr=%i len=%i critical=%u\n", option_nr, option_len, option_nr & 1);
                    if (option_nr & 1) {
                        DEBUG("nanocoap: discarding packet with unknown critical option.\n");
                        return -EBADMSG;
                    }
            }

            pkt_pos += option_len;
        }
    }

    DEBUG("coap pkt parsed. code=%u detail=%u payload_len=%u, 0x%02x\n",
            coap_get_code_class(pkt),
            coap_get_code_detail(pkt),
            pkt->payload_len, hdr->code);

    return 0;
}

ssize_t coap_handle_req(coap_pkt_t *pkt, uint8_t *resp_buf, unsigned resp_buf_len)
{
    if (coap_get_code_class(pkt) != COAP_REQ) {
        DEBUG("coap_handle_req(): not a request.\n");
        return -EBADMSG;
    }

    unsigned method_flag = coap_method2flag(coap_get_code_detail(pkt));

    for (unsigned i = 0; i < coap_resources_numof; i++) {
        if (! (coap_resources[i].methods & method_flag)) {
            continue;
        }

        int res = strcmp((char*)pkt->url, coap_resources[i].path);
        if (res > 0) {
            continue;
        }
        else if (res < 0) {
            break;
        }
        else {
            return coap_resources[i].handler(pkt, resp_buf, resp_buf_len);
        }
    }

    return coap_build_reply(pkt, COAP_CODE_404, resp_buf, resp_buf_len, 0);
}

ssize_t coap_build_reply(coap_pkt_t *pkt, unsigned code,
        uint8_t *rbuf, unsigned rlen, unsigned payload_len)
{
    unsigned tkl = coap_get_token_len(pkt);
    unsigned len = sizeof(coap_hdr_t) + tkl;

    if ((len + payload_len + 1) > rlen) {
        return -ENOSPC;
    }

    coap_build_hdr((coap_hdr_t*)rbuf, COAP_RESP, pkt->token, tkl, code, pkt->hdr->id);
    coap_hdr_set_type((coap_hdr_t*)rbuf, COAP_RESP);
    coap_hdr_set_code((coap_hdr_t*)rbuf, code);

    len += payload_len;

    return len;
}

ssize_t coap_build_hdr(coap_hdr_t *hdr, unsigned type, uint8_t *token, size_t token_len, unsigned code, uint16_t id)
{
    assert(!(type & ~0x3));
    assert(!(token_len & ~0x1f));

    memset(hdr, 0, sizeof(coap_hdr_t));
    hdr->ver_t_tkl = (0x1 << 6) | (type << 4) | token_len;
    hdr->code = code;
    hdr->id = id;

    if (token_len) {
        memcpy(hdr->data, token, token_len);
    }

    return sizeof(coap_hdr_t) + token_len;
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

unsigned coap_put_odelta(uint8_t *buf, unsigned lastonum, unsigned onum, unsigned olen)
{
    unsigned delta = onum - lastonum;
    if (delta < 13) {
        *buf = (uint8_t) ((delta << 4) | olen);
        return 1;
    } else if (delta == 13) {
        *buf++ = (uint8_t) ((13 << 4) | olen);
        *buf = delta - 13;
        return 2;
    } else {
        *buf++ = (uint8_t) ((14 << 4) | olen);
        uint16_t tmp = delta - 269;
        tmp = htons(tmp);
        memcpy(buf, &tmp, 2);
        return 3;
    }
}

size_t coap_put_option(uint8_t *buf, uint16_t lastonum, uint16_t onum, uint8_t *odata, size_t olen)
{
    assert(lastonum <= onum);
    int n = coap_put_odelta(buf, lastonum, onum, olen);
    if(olen) {
        memcpy(buf + n, odata, olen);
        n += olen;
    }
    return n;
}

size_t coap_put_option_ct(uint8_t *buf, uint16_t lastonum, uint16_t content_type)
{
    if (!content_type) {
        return 0;
    }
    if (content_type <= 255) {
        uint8_t tmp = content_type;
        return coap_put_option(buf, lastonum, COAP_OPT_CT, &tmp, sizeof(tmp));
    }
    else {
        return coap_put_option(buf, lastonum, COAP_OPT_CT, (uint8_t*)&content_type, sizeof(content_type));
    }
}

size_t coap_put_option_url(uint8_t *buf, uint16_t lastonum, const char *url)
{
    size_t url_len = strlen(url);
    assert(url_len);

    uint8_t *bufpos = buf;
    char *urlpos = (char*)url;

    while(url_len) {
        size_t part_len;
        urlpos++;
        uint8_t *part_start = (uint8_t*)urlpos;

        while (url_len--) {
            if ((*urlpos == '/') || (*urlpos == '\0')) {
                break;
            }
            urlpos++;
        }

        part_len = (uint8_t*)urlpos - part_start;

        bufpos += coap_put_option(bufpos, lastonum, COAP_OPT_URL, part_start, part_len);
        lastonum = COAP_OPT_URL;
    }

    return bufpos - buf;
}
