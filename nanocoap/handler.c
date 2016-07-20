#include <stdio.h>
#include <string.h>

#include "nanocoap.h"

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
            const char payload[] = "TEST";
            return coap_build_reply(pkt, COAP_CODE_205, buf, len, (uint8_t*)payload, 4);
        default:
            puts("unhandled method");
            return -1;
    }

    return 0;
}

extern ssize_t _well_known_core_handler(coap_pkt_t* pkt, uint8_t *buf, size_t len)
{
    uint8_t *payload = buf + coap_get_total_hdr_len(pkt) + 1;
    uint8_t *bufpos = payload;
    for (int i = 0; i < nanocoap_endpoints_numof; i++) {
        if (i) {
            *bufpos++ = ',';
        }
        *bufpos++ = '<';
        unsigned url_len = strlen(endpoints[i].path);
        memcpy(bufpos, endpoints[i].path, url_len);
        bufpos += url_len;
        *bufpos++ = '>';
    }

    unsigned payload_len = bufpos - payload;

    return coap_build_reply(pkt, COAP_CODE_205, buf, len, NULL, payload_len);
}
