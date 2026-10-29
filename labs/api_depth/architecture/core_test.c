#include "test.h"
#include <limits.h>

struct ac_store
{
    struct ar_record rows[3];
    unsigned reverse;
};

static int get(void *ctx, size_t i, struct ar_record *out)
{
    struct ac_store *s = ctx;
    if (i >= 3)
        return AR_RANGE;
    *out = s->rows[s->reverse ? 2 - i : i];
    return AR_OK;
}

int main(void)
{
    int32_t in[8] = {1, 3, 5}, out[8] = {9, 9, 9};
    CHECK(ar_double(in, 3, out) == AR_OK && out[0] == 2 && out[1] == 6 &&
          out[2] == 10);
    CHECK(ar_double(0, 0, 0) == AR_OK);
    in[1] = INT32_MAX;
    out[0] = 91;
    CHECK(ar_double(in, 3, out) == AR_RANGE && out[0] == 91);
    in[1] = INT32_MIN;
    CHECK(ar_double(in, 3, out) == AR_RANGE && out[0] == 91);
    unsigned enumerated = 0;
    for (unsigned n = 0; n <= 4; ++n)
    {
        unsigned total = 1;
        for (unsigned i = 0; i < n; ++i)
            total *= 3;
        for (unsigned k = 0; k < total; ++k)
        {
            unsigned v = k;
            for (unsigned i = 0; i < n; ++i)
            {
                in[i] = (int32_t)(v % 3) - 1;
                v /= 3;
            }
            CHECK(ar_double(in, n, out) == AR_OK);
            for (unsigned i = 0; i < n; ++i)
                CHECK(out[i] == in[i] + in[i]);
            ++enumerated;
        }
    }
    CHECK(enumerated == 121);
    uint32_t seed = 0x12345678;
    for (unsigned i = 0; i < 10000; ++i)
    {
        seed = seed * 1664525u + 1013904223u;
        int64_t signed_v =
            (int64_t)seed - (seed > INT32_MAX ? INT64_C(4294967296) : 0);
        int32_t v = (int32_t)signed_v, result = 77;
        int status = ar_double(&v, 1, &result);
        int64_t expected = signed_v * 2;
        if (expected < INT32_MIN || expected > INT32_MAX)
            CHECK(status == AR_RANGE && result == 77);
        else
            CHECK(status == AR_OK && result == expected);
    }
    struct ac_store a = {{{1}, {3}, {5}}, 0}, b = {{{5}, {3}, {1}}, 1};
    int32_t x[3], y[3];
    CHECK(ar_process((struct ar_storage){&a, get}, 3, x) == AR_OK);
    CHECK(ar_process((struct ar_storage){&b, get}, 3, y) == AR_OK &&
          !memcmp(x, y, sizeof x));
    for (unsigned version = 1; version <= 2; ++version)
    {
        int32_t values[] = {-32768, -1, 0, 32767};
        for (unsigned i = 0; i < 4; ++i)
        {
            unsigned char wire[5];
            size_t n = version == 1 ? 3 : 5;
            struct ar_record r = {values[i]}, decoded = {99};
            CHECK(ar_encode(version, r, wire, n) == AR_OK);
            CHECK(ar_decode(version, wire, n, &decoded) == AR_OK &&
                  decoded.value == r.value);
            for (size_t split = 0; split < n; ++split)
            {
                struct ar_reader reader = {{0}, 0, version};
                decoded.value = 99;
                CHECK(ar_feed(&reader, wire, split, &decoded) == AR_PENDING &&
                      decoded.value == 99);
                CHECK(ar_feed(&reader, wire + split, n - split, &decoded) ==
                          AR_OK &&
                      decoded.value == r.value);
            }
            wire[n - 1] ^= 1;
            decoded.value = 99;
            CHECK(ar_decode(version, wire, n, &decoded) == AR_INVALID &&
                  decoded.value == 99);
        }
    }
    unsigned char wire[5];
    CHECK(ar_encode(1, (struct ar_record){INT32_MAX}, wire, 3) == AR_RANGE);
    CHECK(ar_encode(2, (struct ar_record){INT32_MIN}, wire, 5) == AR_OK);
    struct ar_record r;
    CHECK(ar_decode(2, wire, 5, &r) == AR_OK && r.value == INT32_MIN);
    struct ar_window w = {0}, replay = {0};
    int32_t sequence[] = {1, 3, 5};
    for (unsigned i = 0; i < 3; ++i)
    {
        int32_t p, q;
        CHECK(ar_window(&w, sequence[i], &p) == AR_OK);
        CHECK(ar_window(&replay, sequence[i], &q) == AR_OK && p == q &&
              p == (i ? sequence[i] + sequence[i - 1] : 1));
    }
    w = (struct ar_window){INT32_MAX, 1};
    struct ar_window old = w;
    int32_t sentinel = 71;
    CHECK(ar_window(&w, 1, &sentinel) == AR_RANGE &&
          !memcmp(&w, &old, sizeof w) && sentinel == 71);
    puts("PASS core 121 finite sequences,10000 full-range "
         "samples,formats,layouts,replay");
}
