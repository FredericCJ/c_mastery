/*
 * Admission-policy check: one wire frame (version 1, payload length 33) meets
 * three parsers. The original parser (capacity 32) and a configured parser
 * restricted to 32 must refuse it; a configured parser with capacity 64 must
 * accept it. The refusal is detected after the 6-byte header has been consumed.
 */
#include "checkpoint_depth.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                    \
    do                                              \
    {                                               \
        if (!(x))                                   \
        {                                           \
            fprintf(stderr, "CPA_POLICY:%s\n", #x); \
            return 41;                              \
        }                                           \
    } while (0)

enum policy_case
{
    POLICY_PAYLOAD_BYTES = 33,
    POLICY_FRAME_BYTES = 43,
    POLICY_HEADER_BYTES = 6,
    POLICY_SEQUENCE = 258,
    POLICY_PAYLOAD_FILL = 7
};

int main(void)
{
    struct cpx_message message = {0};
    message.version = 1;
    message.sequence = POLICY_SEQUENCE;
    message.length = POLICY_PAYLOAD_BYTES;
    memset(message.payload, POLICY_PAYLOAD_FILL, POLICY_PAYLOAD_BYTES);
    unsigned char wire[CPX_WIRE_MAX];
    size_t wire_bytes;
    size_t used;
    CHECK(cpx_protocol_encode(&message, wire, sizeof wire, &wire_bytes) ==
              CPX_P_OK &&
          wire_bytes == POLICY_FRAME_BYTES);
    struct cp_parser original;
    cp_parser_init(&original);
    CHECK(cp_feed(&original, wire, wire_bytes, &used) == CP_REJECT &&
          used == POLICY_HEADER_BYTES && !original.ready);
    struct cpx_parser configured;
    CHECK(cpx_protocol_init(&configured, 32) == CPX_P_OK &&
          cpx_protocol_feed(&configured, wire, wire_bytes, &used) ==
              CPX_P_REJECT &&
          used == POLICY_HEADER_BYTES);
    CHECK(cpx_protocol_init(&configured, 64) == CPX_P_OK &&
          cpx_protocol_feed(&configured, wire, wire_bytes, &used) ==
              CPX_P_FRAME &&
          used == wire_bytes &&
          configured.message.length == POLICY_PAYLOAD_BYTES);
    puts(
        "CPA_POLICY_PASS original32/restricted32 refuse; configured64 accepts version1 length33");
    return 0;
}
