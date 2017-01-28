#include <embUnit/embUnit.h>
#include "nanocoap/nanocoap.h"

/*
 * Parses a basic NON GET request for '/cli/stats'. Includes a 2-byte token.
 */
static void testParseGetReq(void)
{
    coap_pkt_t pdu;

    uint8_t buf[] = {
        0x52, 0x01, 0xd3, 0x06, 0x35, 0x61, 0xb3, 0x63,
        0x6c, 0x69, 0x05, 0x73, 0x74, 0x61, 0x74, 0x73
    };

    int res = coap_parse(&pdu, &buf[0], sizeof(buf));

    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT_EQUAL_INT(COAP_METHOD_GET, coap_get_code(&pdu));
    TEST_ASSERT_EQUAL_INT(2, coap_get_token_len(&pdu));
    TEST_ASSERT_EQUAL_INT(4 + 2, coap_get_total_hdr_len(&pdu));
    TEST_ASSERT_EQUAL_INT(COAP_TYPE_NON, coap_get_type(&pdu));
    TEST_ASSERT_EQUAL_INT(0, pdu.payload_len);
    TEST_ASSERT_EQUAL_STRING("/cli/stats", (char *) &pdu.url[0]);
}

TestRef NanocoapTest_tests(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture(testParseGetReq),
	};
	EMB_UNIT_TESTCALLER(NanocoapTest, NULL, NULL, fixtures);

	return (TestRef)&NanocoapTest;
}
