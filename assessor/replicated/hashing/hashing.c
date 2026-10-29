#include "hashing.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && UINT32_MAX == UINT32_C(0xffffffff),
               "octets/32-bit words required");
#ifndef RPH_MUTATION
#define RPH_MUTATION 0
#endif
#ifndef RPH_SCHEDULE64
#define RPH_SCHEDULE64 0
#endif
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static uint32_t rotate(uint32_t v, unsigned n)
{
    return (v >> n) | (v << (32 - n));
}

static uint32_t s0(uint32_t v)
{
    return rotate(v, 7) ^ rotate(v, 18) ^ (v >> 3);
}

static uint32_t s1(uint32_t v)
{
    return rotate(v, 17) ^ rotate(v, 19) ^ (v >> 10);
}

static void compress(struct rph_sha *s)
{
    uint32_t w[RPH_SCHEDULE64 ? 64 : 16],
        a = s->words[0], b = s->words[1], c = s->words[2], d = s->words[3],
        e = s->words[4], f = s->words[5], g = s->words[6], h = s->words[7];
    unsigned i;
    for (i = 0; i < 16; ++i)
        w[i] = (uint32_t)s->block[4 * i] << 24 |
               (uint32_t)s->block[4 * i + 1] << 16 |
               (uint32_t)s->block[4 * i + 2] << 8 | s->block[4 * i + 3];
    for (i = 0; i < 64; ++i)
    {
        uint32_t word, t1, t2;
        if (RPH_SCHEDULE64)
        {
            if (i >= 16)
                w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
            word = w[i];
        }
        else
        {
            if (i >= 16)
                w[i & 15] = s1(w[(i - 2) & 15]) + w[(i - 7) & 15] +
                            s0(w[(i - 15) & 15]) + w[i & 15];
            word = w[i & 15];
        }
        t1 = h + (rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25)) +
             ((e & f) ^ (~e & g)) + k[i] + word;
        t2 = (rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22)) +
             ((a & b) ^ (a & c) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    s->words[0] += a;
    s->words[1] += b;
    s->words[2] += c;
    s->words[3] += d;
    s->words[4] += e;
    s->words[5] += f;
    s->words[6] += g;
    s->words[7] += h;
}

static int active(const struct rph_sha *s)
{
    return s && s->final == 0 && s->bytes <= RPH_MAX_BYTES && s->used < 64 &&
           s->used == s->bytes % 64;
}

int rph_init(struct rph_sha *s)
{
    static const uint32_t initial[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372,
                                        0xa54ff53a, 0x510e527f, 0x9b05688c,
                                        0x1f83d9ab, 0x5be0cd19};
    unsigned i;
    if (!s)
        return RPH_INVALID;
    for (i = 0; i < 8; ++i)
        s->words[i] = initial[i];
    for (i = 0; i < 64; ++i)
        s->block[i] = 0;
    s->bytes = 0;
    s->used = 0;
    s->final = 0;
    return RPH_OK;
}

int rph_update(struct rph_sha *s, const unsigned char *bytes, size_t n)
{
    size_t i;
    if (!s || (!bytes && n))
        return RPH_INVALID;
    if (!active(s))
        return RPH_STATE;
    if (n > RPH_MAX_BYTES - s->bytes && RPH_MUTATION != 5)
        return RPH_RANGE;
    s->bytes += (uint64_t)n;
    for (i = 0; i < n; ++i)
    {
        s->block[s->used++] = bytes[i];
        if (s->used == 64)
        {
            compress(s);
            s->used = 0;
        }
    }
    return RPH_OK;
}

int rph_final(struct rph_sha *s, unsigned char out[32])
{
    struct rph_sha t;
    uint64_t bits;
    unsigned i;
    if (!s || !out)
        return RPH_INVALID;
    if (!active(s))
        return RPH_STATE;
    t = *s;
    bits = t.bytes * 8;
    t.block[t.used++] = 0x80;
    if (t.used > 56 && RPH_MUTATION != 1)
    {
        while (t.used < 64)
            t.block[t.used++] = 0;
        compress(&t);
        t.used = 0;
    }
    /* Mutant1 safely drops padding when the length field needs another block. */
    while (t.used < 56)
        t.block[t.used++] = 0;
    for (i = 0; i < 8; ++i)
        t.block[56 + i] =
            (unsigned char)(bits >> (RPH_MUTATION == 2 ? 8 * i : 56 - 8 * i));
    compress(&t);
    for (i = 0; i < 32; ++i)
        out[i] = (unsigned char)(t.words[i / 4] >> (24 - 8 * (i % 4)));
    if (RPH_MUTATION != 6)
        s->final = 1;
    rph_clear(&t, sizeof t);
    return RPH_OK;
}

int rph_hash(const unsigned char *bytes, size_t n, unsigned char out[32])
{
    struct rph_sha s;
    int status;
    if (!out || (!bytes && n))
        return RPH_INVALID;
    if (n > RPH_MAX_BYTES)
        return RPH_RANGE;
    (void)rph_init(&s);
    status = rph_update(&s, bytes, n);
    if (status == RPH_OK)
        status = rph_final(&s, out);
    rph_clear(&s, sizeof s);
    return status;
}

int rph_hmac_init(struct rph_hmac *s, const unsigned char *key, size_t n)
{
    struct rph_hmac next;
    unsigned char shortkey[32], pad[64];
    unsigned i;
    if (!s || (!key && n))
        return RPH_INVALID;
    if (n > RPH_MAX_BYTES)
        return RPH_RANGE;
    if (n > 64)
    {
        if (RPH_MUTATION == 3)
            n = 64;
        else
        {
            (void)rph_hash(key, n, shortkey);
            key = shortkey;
            n = 32;
        }
    }
    (void)rph_init(&next.inner);
    (void)rph_init(&next.outer);
    for (i = 0; i < 64; ++i)
        pad[i] = (unsigned char)((i < n ? key[i] : 0) ^ 0x36);
    (void)rph_update(&next.inner, pad, 64);
    for (i = 0; i < 64; ++i)
        pad[i] = (unsigned char)((i < n ? key[i] : 0) ^
                                 (RPH_MUTATION == 4 ? 0x36 : 0x5c));
    (void)rph_update(&next.outer, pad, 64);
    *s = next;
    rph_clear(shortkey, sizeof shortkey);
    rph_clear(pad, sizeof pad);
    rph_clear(&next, sizeof next);
    return RPH_OK;
}

static int hactive(const struct rph_hmac *s)
{
    return s && active(&s->inner) && active(&s->outer) &&
           s->inner.bytes >= 64 && s->outer.bytes == 64;
}

int rph_hmac_update(struct rph_hmac *s, const unsigned char *bytes, size_t n)
{
    if (!s || (!bytes && n))
        return RPH_INVALID;
    if (!hactive(s))
        return RPH_STATE;
    return rph_update(&s->inner, bytes, n);
}

int rph_hmac_final(struct rph_hmac *s, unsigned char out[32])
{
    struct rph_hmac next;
    unsigned char inner[32], result[32];
    unsigned i;
    if (!s || !out)
        return RPH_INVALID;
    if (!hactive(s))
        return RPH_STATE;
    next = *s;
    (void)rph_final(&next.inner, inner);
    (void)rph_update(&next.outer, inner, 32);
    (void)rph_final(&next.outer, result);
    *s = next;
    for (i = 0; i < 32; ++i)
        out[i] = result[i];
    rph_clear(&next, sizeof next);
    rph_clear(inner, sizeof inner);
    rph_clear(result, sizeof result);
    return RPH_OK;
}

int rph_hmac(const unsigned char *key, size_t kn, const unsigned char *bytes,
             size_t n, unsigned char out[32])
{
    struct rph_hmac s;
    int status;
    if (!out || (!key && kn) || (!bytes && n))
        return RPH_INVALID;
    if (kn > RPH_MAX_BYTES || n > RPH_MAX_BYTES - 64)
        return RPH_RANGE;
    if (RPH_MUTATION == 7 && kn <= RPH_MAX_BYTES - n)
    {
        struct rph_sha prefix;
        (void)rph_init(&prefix);
        (void)rph_update(&prefix, key, kn);
        (void)rph_update(&prefix, bytes, n);
        status = rph_final(&prefix, out);
        rph_clear(&prefix, sizeof prefix);
        return status;
    }
    status = rph_hmac_init(&s, key, kn);
    if (status == RPH_OK)
        status = rph_hmac_update(&s, bytes, n);
    if (status == RPH_OK)
        status = rph_hmac_final(&s, out);
    rph_clear(&s, sizeof s);
    return status;
}

int rph_equal(const unsigned char a[32], const unsigned char b[32])
{
    unsigned char different = 0;
    unsigned i;
    if (!a || !b)
        return 0;
    if (RPH_MUTATION == 8)
        return a[0] == b[0];
    for (i = 0; i < 32; ++i)
        different |= (unsigned char)(a[i] ^ b[i]);
    return different == 0;
}

void rph_clear(void *object, size_t n)
{
    volatile unsigned char *p = object;
    size_t i;
    for (i = 0; i < n; ++i)
        p[i] = 0;
}
