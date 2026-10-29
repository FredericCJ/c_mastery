#include "boot_pair.h"
#include "monocypher.h"
#include "monocypher-ed25519.h"
#ifndef FB_GUEST
#include <stdio.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "FB_CHECK:%d:%s\n", __LINE__, #x); \
            return __LINE__;                                   \
        }                                                      \
    } while (0)
#else
#define C(x)                 \
    do                       \
    {                        \
        if (!(x))            \
            return __LINE__; \
    } while (0)
#endif
static void put(unsigned char *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(n >> (8 * i));
    }
}

static int same(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
    {
        if (x[i] != y[i])
        {
            return 0;
        }
    }
    return 1;
}

static void fill(void *a, unsigned char b, size_t n)
{
    unsigned char *p = a;
    for (size_t i = 0; i < n; ++i)
    {
        p[i] = b;
    }
}

static unsigned nibble(char c)
{
    return c >= 'a' ? (unsigned)(c - 'a' + 10) : (unsigned)(c - '0');
}

static void hex(unsigned char *out, const char *s, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = (unsigned char)(16 * nibble(s[2 * i]) + nibble(s[2 * i + 1]));
    }
}

int fb_selftest(void)
{
    /* RFC8032 section7.1 TEST1, independent published expected signature. */
    static const char key_hex[] =
        "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
    static const char signature_hex[] =
        "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b";
    _Static_assert(sizeof key_hex == 65 && sizeof signature_hex == 129,
                   "exact RFC hex lengths");
    unsigned char rfc_key[32], rfc_sig[64];
    hex(rfc_key, key_hex, 32);
    hex(rfc_sig, signature_hex, 64);
    C(crypto_ed25519_check(rfc_sig, rfc_key, (const unsigned char *)"", 0) ==
      0);
    rfc_sig[0] ^= 1;
    C(crypto_ed25519_check(rfc_sig, rfc_key, (const unsigned char *)"", 0) !=
      0);
    struct fb_policy p;
    fill(&p, 0, sizeof p);
    unsigned char secret[2][64];
    for (unsigned c = 0; c < 2; ++c)
    {
        unsigned char seed[32];
        for (unsigned i = 0; i < 32; ++i)
        {
            seed[i] = (unsigned char)(17 * c + i);
        }
        crypto_ed25519_key_pair(secret[c], p.keys[c], seed);
        p.floor[c] = 3;
    }
    p.present = 6;
    p.generation_floor = 5;
    unsigned char bytes[2][608], signature[2][64];
    struct fb_layout layout[2] = {
        {0x08040000u, 0x08060000u, 0x20000000u, 0x20020000u},
        {0x08140000u, 0x08160000u, 0x30000000u, 0x30020000u}};
    struct fb_image image[2], before;
    for (unsigned c = 0; c < 2; ++c)
    {
        fill(bytes[c], 0, 608);
        put(bytes[c], 0x31423646u);
        bytes[c][4] = (unsigned char)(c + 1);
        bytes[c][5] = (unsigned char)c;
        put(bytes[c] + 8, 3);
        put(bytes[c] + 12, 5);
        put(bytes[c] + 16, layout[c].base);
        put(bytes[c] + 20, 576);
        put(bytes[c] + 32, layout[c].stack_high);
        put(bytes[c] + 36, layout[c].base + 513);
        crypto_ed25519_sign(signature[c], secret[c], bytes[c], 608);
        C(fb_admit(&p, layout[c], c, bytes[c], 608, signature[c], &image[c]));
    }
    before = image[0];
    signature[0][0] ^= 1;
    C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]) &&
      same(&image[0], &before, sizeof before));
    signature[0][0] ^= 1;
    for (unsigned i = 0; i < 608; ++i)
    {
        bytes[0][i] ^= 1;
        C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]));
        bytes[0][i] ^= 1;
    }
    put(bytes[0] + 36, layout[0].base + 512);
    crypto_ed25519_sign(signature[0], secret[0], bytes[0], 608);
    C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]));
    put(bytes[0] + 36, layout[0].base + 513);
    crypto_ed25519_sign(signature[0], secret[0], bytes[0], 608);
    p.revoked = 2;
    C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]));
    p.revoked = 0;
    p.floor[0] = 4;
    C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]));
    p.floor[0] = 3;
    p.generation_floor = 6;
    C(!fb_admit(&p, layout[0], 0, bytes[0], 608, signature[0], &image[0]));
    p.generation_floor = 5;
    C(!fb_admit(&p, layout[0], 1, bytes[0], 608, signature[0], &image[0]));
    C(!fb_admit(&p, layout[0], 0, bytes[0], 607, signature[0], &image[0]));
    C(!fb_admit(&p,
                (struct fb_layout){layout[0].base, layout[0].base + 575,
                                   layout[0].stack_low, layout[0].stack_high},
                0, bytes[0], 608, signature[0], &image[0]));
    struct fw_peer peer = {0};
    peer.generation = 5;
    unsigned char record[128], sentinel[128];
    fill(record, 0xA5, 128);
    fill(sentinel, 0xA5, 128);
    C(!fb_joint_record(image, &peer, 9, record) && same(record, sentinel, 128));
    C(fw_peer_step(&peer, 0, 5, FW_HEALTHY) == H7_OK);
    C(fw_peer_step(&peer, 1, 5, FW_FAIL_PEER) == H7_OK);
    C(!fb_joint_record(image, &peer, 9, record) && same(record, sentinel, 128));
    peer = (struct fw_peer){0};
    peer.generation = 5;
    C(fw_peer_step(&peer, 0, 5, FW_HEALTHY) == H7_OK);
    C(fw_peer_step(&peer, 1, 5, FW_HEALTHY) == H7_OK);
    C(fw_peer_step(&peer, 0, 5, FW_CONFIRM) == H7_OK);
    C(fb_joint_record(image, &peer, 9, record) &&
      fb_record_matches(record, image));
    image[1].digest[31] ^= 1;
    C(!fb_record_matches(record, image));
    image[1].digest[31] ^= 1;
    image[1].generation++;
    C(!fb_joint_record(image, &peer, 10, record));
    return 0;
}
#ifndef FB_GUEST
int main(void)
{
    int r = fb_selftest();
    if (!r)
    {
        puts(
            "FB_PASS:actual Ed25519,608authenticated-byte mutations,paired health failure preserves publication");
    }
    return r;
}
#endif
