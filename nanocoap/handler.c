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

const coap_resource_t coap_resources[] = {
    COAP_WELL_KNOWN_CORE_DEFAULT_HANDLER,
    { "/test", COAP_GET, _test_handler },
};

const unsigned coap_resources_numof = sizeof(coap_resources) / sizeof(coap_resources[0]);
