#include <stdio.h>
#include <string.h>

#include "nanocoap.h"

ssize_t _test_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len)
{
    uint8_t *bufpos = buf + coap_get_total_hdr_len(pkt);

    printf("_test_handler()\n");
    printf("coap pkt parsed. code=%u detail=%u payload_len=%u, len=%u 0x%02x\n",
            coap_get_code_class(pkt),
            coap_get_code_detail(pkt),
            pkt->payload_len, (unsigned)len, pkt->hdr->code);

    const char payload[] = "1234";
    *bufpos++ = 0xff;
    memcpy(bufpos, payload, 4);
    return coap_build_reply(pkt, COAP_CODE_205, buf, len, 5);

    return 0;
}

ssize_t _well_known_core_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len)
{
    uint8_t *payload = buf + coap_get_total_hdr_len(pkt);

    uint8_t *bufpos = payload;

    bufpos += coap_put_option_ct(bufpos, 0, COAP_CT_LINK_FORMAT);
    *bufpos++ = 0xff;

    for (unsigned i = 0; i < coap_resources_numof; i++) {
        if (i) {
            *bufpos++ = ',';
        }
        *bufpos++ = '<';
        unsigned url_len = strlen(coap_resources[i].path);
        memcpy(bufpos, coap_resources[i].path, url_len);
        bufpos += url_len;
        *bufpos++ = '>';
    }

    unsigned payload_len = bufpos - payload;

    return coap_build_reply(pkt, COAP_CODE_205, buf, len, payload_len);
}

ssize_t _test_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len);
ssize_t _well_known_core_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len);

const coap_resource_t coap_resources[] = {
    { "/.well-known/core", COAP_GET, _well_known_core_handler },
    { "/test", COAP_GET, _test_handler },
};

const unsigned coap_resources_numof = sizeof(coap_resources) / sizeof(coap_resources[0]);
