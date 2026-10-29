#include "auth.h"
#include "checkpoint_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "CPX_CHECK: %s:%d: %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

static int fail_mac(const unsigned char *k, size_t l, const unsigned char *b,
                    size_t n, unsigned char t[32])
{
    (void)k;
    (void)l;
    (void)b;
    (void)n;
    (void)t;
    return 0;
}

int main(void)
{
    unsigned char key[20], tag[32], expected[32];
    memset(key, 0x0b, sizeof key);
    const char *hex =
        "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";
    for (unsigned i = 0; i < 32; ++i)
    {
        unsigned value;
        CHECK(sscanf(hex + 2 * i, "%2x", &value) == 1);
        expected[i] = (unsigned char)value;
    }
    CHECK(cpx_openssl_mac(key, sizeof key, (const unsigned char *)"Hi There", 8,
                          tag) &&
          cpx_tag_equal(tag, expected));
    struct cpx_auth_state sender = {0}, receiver = {0};
    for (unsigned i = 0; i < 32; ++i)
    {
        sender.key[i] = (unsigned char)i;
    }
    sender.key_id = 7;
    sender.epoch = 19;
    receiver = sender;
    struct cp_message message = {0};
    message.sequence = 258;
    message.length = 3;
    memcpy(message.payload, "abc", 3);
    unsigned char inner[CP_WIRE];
    size_t inner_n;
    CHECK(cp_encode(&message, inner, sizeof inner, &inner_n) == CP_OK);
    unsigned char wire[CPX_AUTH_WIRE], changed[CPX_AUTH_WIRE], output[64];
    size_t n, written = 99;
    memset(output, 0xcc, sizeof output);
    CHECK(cpx_auth_sign(&sender, 1, inner, inner_n, wire, sizeof wire, &n,
                        cpx_openssl_mac) == CPX_AUTH_OK);
    memcpy(changed, wire, n);
    changed[16] = 100;
    CHECK(cpx_auth_open(&receiver, changed, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_FAILED &&
          !receiver.seen && written == 99 && output[0] == 0xcc);
    memcpy(changed, wire, n);
    changed[n - 1] ^= 1;
    CHECK(cpx_auth_open(&receiver, changed, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_FAILED &&
          !receiver.seen);
    CHECK(cpx_auth_open(&receiver, wire, n, output, 1, &written,
                        cpx_openssl_mac) == CPX_AUTH_FULL &&
          !receiver.seen);
    CHECK(cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        fail_mac) == CPX_AUTH_FAILED &&
          !receiver.seen);
    CHECK(cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_OK &&
          written == inner_n && !memcmp(inner, output, inner_n));
    CHECK(cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_REPLAY);
    struct cpx_auth_state wrong = sender;
    wrong.key[0] ^= 1;
    CHECK(cpx_auth_open(&wrong, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_FAILED);
    wrong = sender;
    wrong.epoch++;
    CHECK(cpx_auth_open(&wrong, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_INVALID);
    /* Attacker changes original teaching payload and recomputes its public CRC. */
    memcpy(changed, wire, n);
    changed[26 + 6] ^= 1;
    uint32_t crc = cp_crc32(changed + 27, inner_n - 5);
    for (unsigned i = 0; i < 4; ++i)
    {
        changed[26 + inner_n - 4 + i] = (unsigned char)(crc >> (8 * i));
    }
    wrong = sender;
    CHECK(cpx_auth_open(&wrong, changed, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_FAILED);
    CHECK(cpx_auth_sign(&sender, UINT64_MAX, inner, inner_n, wire, sizeof wire,
                        &n, cpx_openssl_mac) == CPX_AUTH_OK);
    CHECK(cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_OK);
    CHECK(cpx_auth_sign(&sender, 0, inner, inner_n, wire, sizeof wire, &n,
                        cpx_openssl_mac) == CPX_AUTH_OK &&
          cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_REPLAY);
    /* Resetting a volatile replay ledger is observably not durable anti-replay. */
    receiver = sender;
    CHECK(cpx_auth_open(&receiver, wire, n, output, sizeof output, &written,
                        cpx_openssl_mac) == CPX_AUTH_OK);
    puts(
        "CPX_PASS RFC4231 vector, header/payload authentication, replay, refusal and failure ordering");
    return 0;
}
