#include "wire_depth.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static struct rpw_limits all(void)
{
    return (struct rpw_limits){4, 256, 32, 3};
}

static struct rpw_field integer(unsigned tag, uint32_t value)
{
    struct rpw_field f = {0};
    f.tag = tag;
    f.value = value;
    return f;
}

static struct rpw_doc chain(unsigned count)
{
    struct rpw_doc d = {0};
    unsigned i;
    d.count = count;
    for (i = 0; i < count; ++i)
    {
        struct rpw_record *r = &d.records[i];
        r->count = i + 1 < count ? 3 : 2;
        r->fields[0] = integer(1, i + 1);
        r->fields[1] = integer(2, rp_zigzag(-(int32_t)i));
        if (i + 1 < count)
        {
            r->fields[2] = integer(3, i + 1);
            r->fields[2].kind = 2;
        }
    }
    return d;
}

static int canonical(const struct rpw_doc *a, const struct rpw_doc *b)
{
    unsigned char x[256], y[256];
    size_t xn, yn;
    return rpw_encode(a, all(), x, sizeof x, &xn) == 0 &&
           rpw_encode(b, all(), y, sizeof y, &yn) == 0 && xn == yn &&
           !memcmp(x, y, xn);
}

static int nested(void)
{
    struct rpw_doc d = chain(4), got, before;
    unsigned char wire[256], other[256];
    size_t n, m;
    unsigned i;
    CHECK(rpw_encode(&d, all(), wire, sizeof wire, &n) == 0 &&
          rpw_decode(wire, n, all(), &got) == 0 && canonical(&d, &got));
    for (i = 0; i < 4; ++i)
    {
        struct rpw_field f = d.records[i].fields[0];
        d.records[i].fields[0] = d.records[i].fields[1];
        d.records[i].fields[1] = f;
    }
    CHECK(rpw_encode(&d, all(), other, sizeof other, &m) == 0 && n == m &&
          !memcmp(wire, other, n));
    {
        unsigned char raw[] = {16, 1, 8, 150, 1, 120, 255, 255, 255, 255, 15};
        struct rp_message base;
        unsigned char expected[48];
        size_t z;
        CHECK(rp_decode(raw, sizeof raw, &base) == RP_OK &&
              rp_encode(&base, expected, sizeof expected, &z) == RP_OK);
        CHECK(rpw_decode(raw, sizeof raw, all(), &got) == 0 &&
              rpw_encode(&got, all(), wire, sizeof wire, &n) == 0 && n == z &&
              !memcmp(wire, expected, n));
    }
    {
        static const unsigned char invalid[][10] = {
            {8, 128, 0, 16, 1},
            {8, 1, 16, 1, 8, 2},
            {8, 128, 128, 128, 128, 16, 16, 1},
            {8, 128, 128, 128, 128, 128, 16, 1},
            {8, 1, 16, 128}};
        unsigned lengths[] = {5, 6, 8, 8, 4};
        memset(&got, 0xa5, sizeof got);
        memcpy(&before, &got, sizeof got);
        for (i = 0; i < 5; ++i)
        {
            CHECK(rpw_decode(invalid[i], lengths[i], all(), &got) ==
                      RPW_INVALID &&
                  !memcmp(&got, &before, sizeof got));
        }
    }
    {
        int32_t values[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
        for (i = 0; i < 5; ++i)
        {
            CHECK(rp_unzigzag(rp_zigzag(values[i])) == values[i]);
        }
    }
    puts(
        "WIRE_NESTED reordered-records flat-producer varint-extrema duplicates "
        "staged-output");
    return 0;
}

static int bounds(void)
{
    struct rpw_doc d = chain(2), got, before;
    struct rpw_limits l = all();
    unsigned char wire[256], out[256], saved[256];
    size_t n, length;
    unsigned cap;
    CHECK(rpw_encode(&d, l, wire, sizeof wire, &n) == 0);
    memset(&got, 0xa5, sizeof got);
    memcpy(&before, &got, sizeof got);
    l.depth = 1;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_LIMIT &&
          !memcmp(&got, &before, sizeof got));
    l = all();
    l.bytes = 4;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_LIMIT);
    l = all();
    l.fields = 2;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_LIMIT);
    {
        unsigned char bad[] = {8, 1, 16, 1, 26, 1, 8};
        CHECK(rpw_decode(bad, sizeof bad, all(), &got) == RPW_INVALID &&
              !memcmp(&got, &before, sizeof got));
    }
    {
        unsigned char required[] = {8, 1, 16, 1, 40, 4},
                      advisory[] = {8, 1, 16, 1, 48, 255, 255, 255, 255, 15},
                      critical[] = {8, 1, 16, 1, 128, 1, 0};
        CHECK(rpw_decode(required, sizeof required, all(), &got) ==
              RPW_REQUIRED);
        CHECK(rpw_decode(critical, sizeof critical, all(), &got) ==
              RPW_REQUIRED);
        CHECK(rpw_decode(advisory, sizeof advisory, all(), &got) == 0);
    }
    memset(out, 0xa5, sizeof out);
    memcpy(saved, out, sizeof out);
    for (cap = 0; cap < n; ++cap)
    {
        length = 999;
        CHECK(rpw_encode(&d, all(), out, cap, &length) == RPW_FULL &&
              length == 999 && !memcmp(out, saved, sizeof out));
    }
    d.records[0].fields[2].value = 0;
    length = 999;
    CHECK(rpw_encode(&d, all(), out, sizeof out, &length) == RPW_INVALID &&
          length == 999 && !memcmp(out, saved, sizeof out));
    d = chain(2);
    d.records[0].count = 2;
    CHECK(rpw_encode(&d, all(), out, sizeof out, &length) == RPW_INVALID);
    d = chain(1);
    d.records[0].count = 9;
    CHECK(rpw_encode(&d, all(), out, sizeof out, &length) == RPW_INVALID);
    d = (struct rpw_doc){0};
    d.count = 7;
    for (cap = 0; cap < 7; ++cap)
    {
        struct rpw_record *r = &d.records[cap];
        r->fields[r->count++] = integer(1, cap);
        r->fields[r->count++] = integer(2, 0);
        if (cap < 3)
        {
            struct rpw_field f = integer(3, cap * 2 + 1);
            f.kind = 2;
            r->fields[r->count++] = f;
            f.tag = 4;
            ++f.value;
            r->fields[r->count++] = f;
        }
        r->fields[r->count++] = integer(8, 0);
        if (cap < 5)
        {
            r->fields[r->count++] = integer(9, 0);
        }
    }
    CHECK(rpw_encode(&d, all(), wire, sizeof wire, &n) == RPW_OK &&
          rpw_decode(wire, n, all(), &got) == RPW_OK && canonical(&d, &got));
    l = all();
    l.fields = 31;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_LIMIT);
    l = all();
    l.bytes = (unsigned)n;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_OK);
    --l.bytes;
    CHECK(rpw_decode(wire, n, l, &got) == RPW_LIMIT);
    d = chain(1);
    before = d;
    l = all();
    l.bytes = 4;
    CHECK(rpw_update(&d, l, UINT32_MAX, INT32_MIN) == RPW_LIMIT &&
          !memcmp(&d, &before, sizeof d));
    puts("WIRE_LIMITS depth bytes fields required-feature unknown-critical "
         "capacities invalid-arena");
    puts("WIRE_EXACT budgets32fields exact-byte-length binary-branch-tree");
    return 0;
}

static int streaming(void)
{
    struct rpw_doc d = chain(4), got, before;
    struct rpw_stream s;
    unsigned char wire[578];
    size_t n, at, used, part;
    unsigned chunk;
    int result;
    CHECK(rpw_frame(&d, all(), wire, 289, &n) == 0);
    memcpy(wire + n, wire, n);
    for (chunk = 1; chunk <= n; ++chunk)
    {
        CHECK(rpw_stream_init(&s, all()) == 0);
        at = 0;
        while (at < n)
        {
            part = n - at < chunk ? n - at : chunk;
            result = rpw_feed(&s, wire + at, part, &used, &got);
            CHECK(used == part);
            at += used;
            CHECK(result == (at == n ? RPW_OK : RPW_MORE));
        }
        CHECK(canonical(&got, &d) && rpw_eof(&s) == 0);
    }
    CHECK(rpw_stream_init(&s, all()) == 0 &&
          rpw_feed(&s, wire, 2 * n, &used, &got) == 0 && used == n &&
          canonical(&got, &d));
    CHECK(rpw_feed(&s, wire + n, n, &used, &got) == RPW_STATE);
    for (at = 0; at < n; ++at)
    {
        CHECK(rpw_stream_init(&s, all()) == 0);
        memset(&got, 0xa5, sizeof got);
        memcpy(&before, &got, sizeof got);
        CHECK(rpw_feed(&s, wire, at, &used, &got) == RPW_MORE && used == at &&
              !memcmp(&got, &before, sizeof got));
        CHECK(rpw_eof(&s) == RPW_INVALID && rpw_eof(&s) == RPW_STATE);
    }
    CHECK(rpw_stream_init(&s, all()) == 0 &&
          rpw_feed(&s, wire, 3, &used, &got) == RPW_MORE &&
          rpw_cancel(&s) == 0 && s.used == 0 &&
          rpw_feed(&s, wire, n, &used, &got) == RPW_STATE);
    CHECK(rpw_stream_init(&s, all()) == 0);
    {
        unsigned char bad = 9;
        CHECK(rpw_feed(&s, &bad, 1, &used, &got) == RPW_INVALID && used == 1);
    }
    {
        struct rpw_limits l = all();
        l.bytes = 4;
        CHECK(rpw_stream_init(&s, l) == 0 &&
              rpw_feed(&s, wire, n, &used, &got) == RPW_LIMIT && s.phase == 2);
    }
    printf(
        "WIRE_STREAM chunks%zu truncations%zu exact-terminal consumed-prefix "
        "cancel bounded-ownership\n",
        n, n);
    return 0;
}

static int evolution(void)
{
    struct rpw_doc d = chain(1), got;
    struct rp_message projected;
    unsigned char before[256], after[256], lost[48];
    size_t n, m, z;
    struct rpw_field unknown = {0};
    unknown.tag = 8;
    unknown.kind = 1;
    unknown.length = 4;
    unknown.bytes[0] = 16;
    unknown.bytes[1] = 1;
    unknown.bytes[2] = 8;
    unknown.bytes[3] = 1;
    d.records[0].fields[2] = unknown;
    d.records[0].count = 3;
    CHECK(rpw_encode(&d, all(), before, sizeof before, &n) == 0 &&
          rpw_update(&d, all(), 9, -7) == 0 &&
          rpw_encode(&d, all(), after, sizeof after, &m) == 0 && m == n &&
          d.records[0].count == 3 &&
          !memcmp(d.records[0].fields[2].bytes, unknown.bytes, 4));
    CHECK(rpw_decode(after, m, all(), &got) == 0 && canonical(&got, &d));
    CHECK(rpw_project(&d, all(), &projected) == 0 &&
          rp_encode(&projected, lost, sizeof lost, &z) == RP_OK && z < m);
    {
        struct rpw_doc other = d;
        unsigned char tmp = other.records[0].fields[2].bytes[0];
        other.records[0].fields[2].bytes[0] =
            other.records[0].fields[2].bytes[2];
        other.records[0].fields[2].bytes[2] = tmp;
        CHECK(!canonical(&other, &d));
    }
    CHECK(rpw_binary64(UINT64_C(0x8000000000000000)) == 0 &&
          rpw_binary64(0) == 0 &&
          rpw_binary64(UINT64_C(0xfff0000000000001)) ==
              UINT64_C(0x7ff8000000000000));
    CHECK(rpw_binary64(UINT64_C(0x7ff0000000000000)) ==
              UINT64_C(0x7ff0000000000000) &&
          rpw_binary64(UINT64_C(0x3ff0000000000000)) ==
              UINT64_C(0x3ff0000000000000));
    {
        unsigned char composed[] = {0xc3, 0xa9},
                      decomposed[] = {'e', 0xcc, 0x81},
                      overlong[] = {0xe0, 0x80, 0x80},
                      surrogate[] = {0xed, 0xa0, 0x80},
                      beyond[] = {0xf4, 0x90, 0x80, 0x80};
        CHECK(rpw_utf8(composed, sizeof composed) == 0 &&
              rpw_utf8(decomposed, sizeof decomposed) == 0 &&
              sizeof composed != sizeof decomposed);
        CHECK(rpw_utf8(overlong, sizeof overlong) == RPW_INVALID &&
              rpw_utf8(surrogate, sizeof surrogate) == RPW_INVALID &&
              rpw_utf8(beyond, sizeof beyond) == RPW_INVALID &&
              rpw_utf8(NULL, 0) == 0);
    }
    puts("WIRE_EVOLUTION retained-opaque reconstruction-loss "
         "unknown-nesting-ambiguity signed-zero NaN exact-UTF8");
    return 0;
}

static int oracle(void)
{
    unsigned depth, bytes, fields, supported;
    char hex[520];
    while (scanf("%u %u %u %u %519s", &depth, &bytes, &fields, &supported,
                 hex) == 5)
    {
        unsigned char raw[256], out[256];
        struct rpw_doc d;
        struct rpw_limits l = {depth, bytes, fields, supported};
        size_t n = 0, z = 0, i;
        int r;
        unsigned x;
        if (strcmp(hex, "-"))
        {
            CHECK(strlen(hex) % 2 == 0 && strlen(hex) <= 512);
            for (i = 0; hex[i]; i += 2)
            {
                CHECK(sscanf(hex + i, "%2x", &x) == 1);
                raw[n++] = (unsigned char)x;
            }
        }
        r = rpw_decode(raw, n, l, &d);
        if (!r)
        {
            r = rpw_encode(&d, l, out, sizeof out, &z);
        }
        printf("%d ", r);
        if (!r)
        {
            for (i = 0; i < z; ++i)
            {
                printf("%02x", out[i]);
            }
        }
        else
        {
            putchar('-');
        }
        putchar('\n');
    }
    return 0;
}

static int normalization_oracle(void)
{
    uint64_t bits;
    char hex[520];
    while (scanf("%" SCNx64 " %519s", &bits, hex) == 2)
    {
        unsigned char raw[256];
        size_t n = 0, i;
        unsigned x;
        if (strcmp(hex, "-"))
        {
            CHECK(strlen(hex) % 2 == 0 && strlen(hex) <= 512);
            for (i = 0; hex[i]; i += 2)
            {
                CHECK(sscanf(hex + i, "%2x", &x) == 1);
                raw[n++] = (unsigned char)x;
            }
        }
        printf("%016" PRIx64 " %d\n", rpw_binary64(bits), rpw_utf8(raw, n));
    }
    return 0;
}

static int metrics(void)
{
    struct rpw_doc d = chain(4);
    unsigned char raw[256], framed[289];
    size_t n, m;
    unsigned i, sample, mode;
    double samples[2][7];
    clock_t start = clock(), end;
    CHECK(start != (clock_t)-1);
    for (i = 0; i < 10000; ++i)
    {
        struct rpw_doc next;
        CHECK(rpw_encode(&d, all(), raw, sizeof raw, &n) == 0 &&
              rpw_decode(raw, n, all(), &next) == 0);
    }
    end = clock();
    CHECK(end != (clock_t)-1 && end >= start &&
          rpw_frame(&d, all(), framed, sizeof framed, &m) == 0);
    for (mode = 0; mode < 2; ++mode)
    {
        for (sample = 0; sample < 7; ++sample)
        {
            clock_t begin = clock(), finish;
            CHECK(begin != (clock_t)-1);
            for (i = 0; i < 5000; ++i)
            {
                struct rpw_stream s;
                struct rpw_doc next;
                size_t at = 0, used;
                CHECK(rpw_stream_init(&s, all()) == 0);
                while (at < m)
                {
                    size_t take = mode ? 1 : m - at;
                    int r = rpw_feed(&s, framed + at, take, &used, &next);
                    CHECK(used == take);
                    at += used;
                    CHECK(r == (at == m ? RPW_OK : RPW_MORE));
                }
                CHECK(next.count == d.count &&
                      next.records[0].fields[0].value == 1 &&
                      rpw_eof(&s) == RPW_OK);
            }
            finish = clock();
            CHECK(finish != (clock_t)-1 && finish >= begin);
            samples[mode][sample] = (double)(finish - begin) / CLOCKS_PER_SEC;
        }
    }
    printf("{\"document_size\":%zu,\"stream_size\":%zu,\"depth\":4,\"byte_"
           "budget\":256,\"field_budget\":32,\"nested_bytes\":%zu,\"framed_"
           "bytes\":%zu,\"runs\":10000,\"cpu_seconds\":%.9f,\"feed_runs\":5000,"
           "\"bulk_cpu_seconds\":[",
           sizeof d, sizeof(struct rpw_stream), n, m,
           (double)(end - start) / CLOCKS_PER_SEC);
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", samples[0][sample]);
    }
    printf("],\"byte_cpu_seconds\":[");
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", samples[1][sample]);
    }
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "wire_nested"))
    {
        return nested();
    }
    if (!strcmp(argv[1], "wire_limits"))
    {
        return bounds();
    }
    if (!strcmp(argv[1], "wire_stream"))
    {
        return streaming();
    }
    if (!strcmp(argv[1], "wire_evolution"))
    {
        return evolution();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    if (!strcmp(argv[1], "normalization_oracle"))
    {
        return normalization_oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
