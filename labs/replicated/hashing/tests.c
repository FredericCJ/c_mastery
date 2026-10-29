#include "hashing.h"
#include "replicated.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef RPH_SCHEDULE64
#define RPH_SCHEDULE64 0
#endif
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static void hex(const unsigned char *p, char out[65])
{
    static const char h[] = "0123456789abcdef";
    unsigned i;
    for (i = 0; i < 32; ++i)
    {
        out[2 * i] = h[p[i] >> 4];
        out[2 * i + 1] = h[p[i] & 15];
    }
    out[64] = 0;
}

static int digest_is(const unsigned char p[32], const char *want)
{
    char got[65];
    hex(p, got);
    return !strcmp(got, want);
}

static int sha_stream(void)
{
    unsigned char data[257], a[32], b[32];
    struct rph_sha s;
    size_t n, split, i;
    CHECK(rph_hash(NULL, 0, a) == RPH_OK);
    CHECK(digest_is(
        a, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    CHECK(rph_hash((const unsigned char *)"abc", 3, a) == RPH_OK);
    CHECK(digest_is(
        a, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    for (i = 0; i < sizeof data; ++i)
    {
        data[i] = (unsigned char)(i * 37 + 11);
    }
    for (n = 0; n <= sizeof data; ++n)
    {
        CHECK(rph_hash(data, n, a) == RPH_OK);
        if (n <= 65)
        {
            CHECK(rp_sha256(data, n, b) == RP_OK);
            CHECK(rph_equal(a, b));
        }
        for (split = 0; split <= n; ++split)
        {
            CHECK(rph_init(&s) == RPH_OK);
            CHECK(rph_update(&s, data, split) == RPH_OK);
            CHECK(rph_update(&s, NULL, 0) == RPH_OK);
            CHECK(rph_update(&s, data + split, n - split) == RPH_OK);
            CHECK(rph_final(&s, b) == RPH_OK);
            CHECK(rph_equal(a, b));
        }
    }
    puts("HASH_SPLITS 33411");
    return 0;
}

static int hmac_test(void)
{
    unsigned char key[131], data[150], a[32], b[32];
    struct rph_hmac s;
    unsigned i;
    memset(key, 0x0b, 20);
    CHECK(rph_hmac(key, 20, (const unsigned char *)"Hi There", 8, a) == RPH_OK);
    CHECK(digest_is(
        a, "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"));
    CHECK(rph_hmac((const unsigned char *)"Jefe", 4,
                   (const unsigned char *)"what do ya want for nothing?", 28,
                   a) == RPH_OK);
    CHECK(digest_is(
        a, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"));
    memset(key, 0xaa, 131);
    CHECK(rph_hmac(
              key, 131,
              (const unsigned char
                   *)"Test Using Larger Than Block-Size Key - Hash Key First",
              54, a) == RPH_OK);
    CHECK(digest_is(
        a, "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"));
    for (i = 0; i < 150; ++i)
    {
        data[i] = (unsigned char)i;
    }
    CHECK(rph_hmac(key, 131, data, 150, a) == RPH_OK);
    CHECK(rph_hmac_init(&s, key, 131) == RPH_OK);
    /* The caller may reuse key/input bytes once each update returns. */
    memset(key, 0, 131);
    for (i = 0; i < 150; ++i)
    {
        CHECK(rph_hmac_update(&s, data + i, 1) == RPH_OK);
    }
    CHECK(rph_hmac_final(&s, b) == RPH_OK);
    CHECK(rph_equal(a, b));
    for (i = 0; i < 32; ++i)
    {
        b[i] ^= 1;
        CHECK(!rph_equal(a, b));
        b[i] ^= 1;
    }
    puts("HMAC_VECTORS 3");
    return 0;
}

static int lifecycle(void)
{
    struct rph_sha s;
    struct rph_hmac h;
    unsigned char old[sizeof h], out[32], saved[32], byte = 7;
    unsigned i;
    CHECK(rph_init(&s) == RPH_OK);
    memcpy(old, &s, sizeof s);
    CHECK(rph_update(&s, NULL, 1) == RPH_INVALID);
    CHECK(!memcmp(old, &s, sizeof s));
    CHECK(rph_final(&s, NULL) == RPH_INVALID);
    CHECK(!memcmp(old, &s, sizeof s));
    /* Structurally valid test injection represents the last permitted byte.
       It verifies admission arithmetic, not the hash of an unprocessed huge
       stream. */
    s.bytes = RPH_MAX_BYTES;
    s.used = (unsigned)(s.bytes % 64);
    memcpy(old, &s, sizeof s);
    CHECK(rph_update(&s, &byte, 1) == RPH_RANGE);
    CHECK(!memcmp(old, &s, sizeof s));
    CHECK(rph_update(&s, NULL, 0) == RPH_OK);
    CHECK(!memcmp(old, &s, sizeof s));
    CHECK(rph_init(&s) == RPH_OK);
    CHECK(rph_update(&s, &byte, 1) == RPH_OK);
    CHECK(rph_final(&s, out) == RPH_OK);
    memcpy(old, &s, sizeof s);
    memcpy(saved, out, 32);
    CHECK(rph_update(&s, &byte, 1) == RPH_STATE);
    CHECK(rph_final(&s, out) == RPH_STATE);
    CHECK(!memcmp(old, &s, sizeof s) && !memcmp(saved, out, 32));
    CHECK(rph_hmac_init(&h, NULL, 0) == RPH_OK);
    CHECK(rph_hmac_update(&h, NULL, 0) == RPH_OK);
    h.inner.bytes = RPH_MAX_BYTES;
    h.inner.used = (unsigned)(RPH_MAX_BYTES % 64);
    memcpy(old, &h, sizeof h);
    CHECK(rph_hmac_update(&h, &byte, 1) == RPH_RANGE);
    CHECK(!memcmp(old, &h, sizeof h));
    CHECK(rph_hmac_init(&h, &byte, 1) == RPH_OK);
    CHECK(rph_hmac_final(&h, out) == RPH_OK);
    memcpy(old, &h, sizeof h);
    memcpy(saved, out, 32);
    CHECK(rph_hmac_final(&h, out) == RPH_STATE);
    CHECK(rph_hmac_update(&h, &byte, 1) == RPH_STATE);
    CHECK(!memcmp(old, &h, sizeof h) && !memcmp(saved, out, 32));
    rph_clear(&h, sizeof h);
    for (i = 0; i < sizeof h; ++i)
    {
        CHECK(((unsigned char *)&h)[i] == 0);
    }
    return 0;
}

static int oracle(void)
{
    unsigned n, kn, chunk, salt, i;
    unsigned char data[8193], key[257], a[32], b[32];
    char x[65], y[65];
    struct rph_sha s;
    struct rph_hmac h;
    while (scanf("%u %u %u %u", &n, &kn, &chunk, &salt) == 4)
    {
        CHECK(n <= 8193 && kn <= 257 && chunk > 0 && chunk <= 8193);
        for (i = 0; i < n; ++i)
        {
            data[i] = (unsigned char)(i * 37 + salt);
        }
        for (i = 0; i < kn; ++i)
        {
            key[i] = (unsigned char)(i * 13 + salt * 3);
        }
        CHECK(rph_init(&s) == RPH_OK);
        CHECK(rph_hmac_init(&h, key, kn) == RPH_OK);
        for (i = 0; i < n;)
        {
            unsigned take = n - i < chunk ? n - i : chunk;
            CHECK(rph_update(&s, data + i, take) == RPH_OK);
            CHECK(rph_hmac_update(&h, data + i, take) == RPH_OK);
            i += take;
        }
        CHECK(rph_final(&s, a) == RPH_OK);
        CHECK(rph_hmac_final(&h, b) == RPH_OK);
        hex(a, x);
        hex(b, y);
        printf("%s %s\n", x, y);
    }
    return 0;
}

static void resume_digest(struct rph_sha *s, const unsigned char tag[32],
                          uint64_t prefix_bytes)
{
    unsigned i;
    (void)rph_init(s);
    for (i = 0; i < 8; ++i)
    {
        s->words[i] = (uint32_t)tag[4 * i] << 24 |
                      (uint32_t)tag[4 * i + 1] << 16 |
                      (uint32_t)tag[4 * i + 2] << 8 | tag[4 * i + 3];
    }
    s->bytes = prefix_bytes;
    s->used = 0;
}

static int security(void)
{
    const unsigned char key[8] = {'l', 'a', 'b', '-', 'k', 'e', 'y', '!'},
                        original[8] = {'a', 'm', 'o', 'u', 'n', 't', '=', '1'},
                        extra[8] = {'&', 'a', 'd', 'm', 'i', 'n', '=', '1'};
    unsigned char request[64] = {0}, bad[32], forged[32], target[32], tag[32],
                  hforged[32];
    struct rph_sha s;
    unsigned i;
    /* Original teaching verifier: SHA256(secret || request). The attacker has
       only the returned tag and knows this lab key has eight bytes. */
    CHECK(rph_init(&s) == RPH_OK);
    CHECK(rph_update(&s, key, 8) == RPH_OK);
    CHECK(rph_update(&s, original, 8) == RPH_OK);
    CHECK(rph_final(&s, tag) == RPH_OK);
    memcpy(request, original, 8);
    request[8] = 0x80;
    request[55] = 128;
    memcpy(request + 56, extra, 8);
    resume_digest(&s, tag, 64);
    CHECK(rph_update(&s, extra, 8) == RPH_OK);
    CHECK(rph_final(&s, forged) == RPH_OK);
    CHECK(rph_init(&s) == RPH_OK);
    CHECK(rph_update(&s, key, 8) == RPH_OK);
    CHECK(rph_update(&s, request, 64) == RPH_OK);
    CHECK(rph_final(&s, target) == RPH_OK);
    CHECK(rph_equal(forged, target));
    /* Identical continuation applied to a genuine HMAC tag must fail. */
    CHECK(rph_hmac(key, 8, original, 8, tag) == RPH_OK);
    resume_digest(&s, tag, 64);
    CHECK(rph_update(&s, extra, 8) == RPH_OK);
    CHECK(rph_final(&s, hforged) == RPH_OK);
    CHECK(rph_hmac(key, 8, request, 64, target) == RPH_OK);
    CHECK(!rph_equal(hforged, target));
    for (i = 0; i < 256; ++i)
    {
        memcpy(bad, target, 32);
        bad[i / 8] ^= (unsigned char)(1u << (i % 8));
        CHECK(!rph_equal(bad, target));
    }
    puts("LOCAL_EXTENSION prefix-accepted=1 hmac-accepted=0 "
         "bit-changes-rejected=256");
    return 0;
}

static int lengths(void)
{
    const uint64_t counts[] = {UINT64_C(536870913), UINT64_C(4294967351),
                               RPH_MAX_BYTES};
    struct rph_sha s;
    unsigned i, j;
    unsigned char out[32];
    char text[65];
    for (i = 0; i < 3; ++i)
    {
        CHECK(rph_init(&s) == RPH_OK);
        s.bytes = counts[i];
        s.used = (unsigned)(s.bytes % 64);
        for (j = 0; j < s.used; ++j)
        {
            s.block[j] = (unsigned char)(j * 19 + 7);
        }
        CHECK(rph_final(&s, out) == RPH_OK);
        hex(out, text);
        puts(text);
    }
    return 0;
}

static int clone_test(void)
{
    struct rph_sha prefix, left, right;
    unsigned char a[32], b[32], expected[32];
    CHECK(rph_init(&prefix) == RPH_OK);
    CHECK(rph_update(&prefix, (const unsigned char *)"pre", 3) == RPH_OK);
    left = prefix;
    right = prefix;
    CHECK(rph_update(&left, (const unsigned char *)"left", 4) == RPH_OK);
    CHECK(rph_update(&right, (const unsigned char *)"right", 5) == RPH_OK);
    CHECK(rph_final(&left, a) == RPH_OK);
    CHECK(rph_hash((const unsigned char *)"preleft", 7, expected) == RPH_OK);
    CHECK(rph_equal(a, expected));
    CHECK(rph_final(&right, b) == RPH_OK);
    CHECK(rph_hash((const unsigned char *)"preright", 8, expected) == RPH_OK);
    CHECK(rph_equal(b, expected));
    CHECK(!rph_equal(a, b));
    CHECK(prefix.bytes == 3 && prefix.final == 0);
    return 0;
}

static int metrics(void)
{
    unsigned char data[8193], out[32];
    const size_t sizes[] = {0, 55, 56, 64, 65, 1024, 8193};
    size_t i, j;
    unsigned checksum = 0;
    struct timespec start, end;
    memset(data, 0x61, sizeof data);
    printf(
        "{\"sha_size\":%zu,\"hmac_size\":%zu,\"schedule_words\":%u,\"samples\":[",
        sizeof(struct rph_sha), sizeof(struct rph_hmac),
        RPH_SCHEDULE64 ? 64 : 16);
    for (i = 0; i < sizeof sizes / sizeof sizes[0]; ++i)
    {
        CHECK(timespec_get(&start, TIME_UTC) == TIME_UTC);
        for (j = 0; j < 1000; ++j)
        {
            CHECK(rph_hash(data, sizes[i], out) == RPH_OK);
            checksum += out[j % 32];
        }
        CHECK(timespec_get(&end, TIME_UTC) == TIME_UTC);
        printf("%s{\"bytes\":%zu,\"repeats\":1000,\"seconds\":%.9f}",
               i ? "," : "", sizes[i],
               (double)(end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1e9);
    }
    printf("],\"checksum\":%u}\n", checksum);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "sha_stream"))
    {
        return sha_stream();
    }
    if (!strcmp(argv[1], "hash_hmac"))
    {
        return hmac_test();
    }
    if (!strcmp(argv[1], "hash_lifecycle"))
    {
        return lifecycle() || clone_test();
    }
    if (!strcmp(argv[1], "hash_security"))
    {
        return security();
    }
    if (!strcmp(argv[1], "lengths"))
    {
        return lengths();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
