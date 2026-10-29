#include "evolution_record.h"
#include "target.h"
#include "arch.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "ADO_CHECK %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                         \
        }                                                                     \
    } while (0)
static unsigned char key[32];

static struct ade_record sentinel(void)
{
    struct ade_record r = {0};
    r.count = 1;
    r.fields[0].tag = 99;
    r.fields[0].length = 1;
    r.fields[0].bytes[0] = 77;
    return r;
}

static int same(struct ade_record a, struct ade_record b)
{
    if (a.count != b.count)
        return 0;
    for (unsigned i = 0; i < ADE_FIELDS; i++)
    {
        if (a.fields[i].tag != b.fields[i].tag ||
            a.fields[i].length != b.fields[i].length)
            return 0;
        for (unsigned j = 0; j < ADE_PAYLOAD; j++)
            if (a.fields[i].bytes[j] != b.fields[i].bytes[j])
                return 0;
    }
    return 1;
}

static int records(void)
{
    unsigned char wire[] = {4, 9, 1, 4, 1, 2, 0, 7, 9, 1, 5, 3, 0},
                  raw[ADE_BYTES], canonical[ADE_BYTES];
    struct ade_record record;
    size_t n = 0;
    CHECK(ade_read(wire, sizeof wire, &record) == ADE_OK);
    CHECK(ade_write(&record, 0, raw, sizeof raw, &n) == ADE_OK &&
          n == sizeof wire && !memcmp(raw, wire, n));
    const unsigned char sorted[] = {4, 1, 2, 0, 7, 3, 0, 9, 1, 4, 9, 1, 5};
    CHECK(ade_write(&record, 1, canonical, sizeof canonical, &n) == ADE_OK &&
          n == sizeof sorted && !memcmp(canonical, sorted, n));
    wire[3] = 99;
    CHECK(record.fields[0].bytes[0] == 4);
    struct ade_record again;
    CHECK(ade_read(canonical, n, &again) == ADE_OK);
    CHECK(ade_write(&again, 1, raw, sizeof raw, &n) == ADE_OK &&
          !memcmp(raw, canonical, n));
    unsigned char duplicate[] = {2, 1, 2, 0, 1, 1, 2, 0, 2};
    again = sentinel();
    struct ade_record saved = again;
    CHECK(ade_read(duplicate, sizeof duplicate, &again) == ADE_INVALID &&
          same(again, saved));
    for (size_t cut = 0; cut < sizeof sorted; cut++)
    {
        again = saved;
        CHECK(ade_read(sorted, cut, &again) == ADE_INVALID &&
              same(again, saved));
    }
    unsigned char trailing[sizeof sorted + 1];
    memcpy(trailing, sorted, sizeof sorted);
    trailing[sizeof sorted] = 0;
    CHECK(ade_read(trailing, sizeof trailing, &again) == ADE_INVALID &&
          same(again, saved));
    for (size_t cap = 0; cap < sizeof sorted; cap++)
    {
        memset(raw, 0xa5, sizeof raw);
        n = 99;
        CHECK(ade_write(&record, 1, raw, cap, &n) == ADE_CAPACITY && n == 99);
        for (size_t j = 0; j < sizeof raw; j++)
            CHECK(raw[j] == 0xa5);
    }
    /* Actual codec.records producer inside opaque fields, two independent editions. */
    for (unsigned version = 1; version <= 2; version++)
    {
        struct ade_record source = {0};
        source.count = 1;
        source.fields[0].tag = 3;
        source.fields[0].length = version == 1 ? 3 : 5;
        struct ar_record value = {version == 1 ? -32768 : INT32_MIN},
                         decoded = {0};
        CHECK(ar_encode(version, value, source.fields[0].bytes,
                        source.fields[0].length) == AR_OK);
        CHECK(ade_write(&source, 0, raw, sizeof raw, &n) == ADE_OK &&
              ade_read(raw, n, &again) == ADE_OK);
        CHECK(ade_write(&again, 1, canonical, sizeof canonical, &n) == ADE_OK &&
              ade_read(canonical, n, &again) == ADE_OK);
        CHECK(ar_decode(version, again.fields[0].bytes, again.fields[0].length,
                        &decoded) == AR_OK &&
              decoded.value == value.value);
    }
    puts("ADE_RECORD stable-duplicates actual-cursor-codec owned-relay");
    return 0;
}

static int authentication(void)
{
    unsigned char base[] = {1, 1, 2, 0, 7},
                  attack[] = {3, 9, 1, 0, 1, 2, 0, 7, 9, 1, 1};
    struct ade_record a, b;
    unsigned char ta[32], tb[32], wrong[32];
    CHECK(ade_read(base, sizeof base, &a) == ADE_OK &&
          ade_read(attack, sizeof attack, &b) == ADE_OK);
    CHECK(ade_tag(&a, key, ta) == ADE_OK && ade_tag(&b, key, tb) == ADE_OK &&
          !memcmp(ta, ta, 32) && memcmp(ta, tb, 32) != 0);
    CHECK(ade_check(&b, key, tb) == ADE_OK);
    memcpy(wrong, tb, 32);
    wrong[31] ^= 1;
    CHECK(ade_check(&b, key, wrong) == ADE_AUTH);
    struct ade_record reverse = b;
    reverse.fields[0] = b.fields[2];
    reverse.fields[2] = b.fields[0];
    CHECK(ade_tag(&reverse, key, wrong) == ADE_OK &&
          memcmp(tb, wrong, 32) != 0);
    struct ade_record reorder = b;
    reorder.fields[0] = b.fields[1];
    reorder.fields[1] = b.fields[0];
    CHECK(ade_tag(&reorder, key, wrong) == ADE_OK && !memcmp(tb, wrong, 32));
    puts("ADE_AUTH all-unknown-occurrences stable-canonical-transcript");
    return 0;
}

static int policy(void)
{
    unsigned char base[] = {1, 1, 2, 0, 7},
                  attack[] = {3, 9, 1, 0, 1, 2, 0, 7, 9, 1, 1};
    struct ade_record a, b;
    unsigned char tag[32];
    struct ade_service s = {10, 0};
    CHECK(ade_read(base, sizeof base, &a) == ADE_OK &&
          ade_read(attack, sizeof attack, &b) == ADE_OK &&
          ade_tag(&a, key, tag) == ADE_OK);
    CHECK(ade_vulnerable(&b, key, tag, &s) == ADE_OK && s.total == 17 &&
          s.privileged == 1);
    s = (struct ade_service){10, 0};
    CHECK(ade_apply(&b, key, tag, &s) == ADE_AUTH && s.total == 10 &&
          s.privileged == 0);
    CHECK(ade_tag(&b, key, tag) == ADE_OK);
    CHECK(ade_apply(&b, key, tag, &s) == ADE_POLICY && s.total == 10 &&
          s.privileged == 0);
    CHECK(ade_tag(&a, key, tag) == ADE_OK &&
          ade_apply(&a, key, tag, &s) == ADE_OK && s.total == 17 &&
          s.privileged == 0);
    s = (struct ade_service){UINT32_MAX - 6, 7};
    CHECK(ade_apply(&a, key, tag, &s) == ADE_RANGE &&
          s.total == UINT32_MAX - 6 && s.privileged == 7);
    puts(
        "ADE_POLICY local-privileged-counter witness authenticated-not-authorized");
    return 0;
}

static int baseline(void)
{
    unsigned char input[257], output[257];
    for (size_t i = 0; i < 255; i++)
        input[i + 2] = (unsigned char)i;
    input[0] = 9;
    input[1] = 255;
    struct ad_evolving value;
    size_t used = 99;
    CHECK(ad_evolve_read(input, sizeof input, &value));
    CHECK(ad_evolve_write(value, output, sizeof output, &used) &&
          used == sizeof input && !memcmp(input, output, used));
    unsigned char malformed[] = {1, 1, 7};
    CHECK(!ad_evolve_read(malformed, sizeof malformed, &value));
    unsigned char borrowed[] = {9, 1, 4};
    CHECK(ad_evolve_read(borrowed, sizeof borrowed, &value));
    borrowed[2] = 8;
    CHECK(value.payload[0] == 8);
    for (size_t cut = 0; cut < sizeof input; cut++)
        CHECK(!ad_evolve_read(input, cut, &value));
    unsigned char critical[] = {128, 0};
    CHECK(!ad_evolve_read(critical, sizeof critical, &value));
    unsigned char empty[] = {9, 0};
    CHECK(ad_evolve_read(empty, 2, &value) && value.length == 0);
    puts("ADE_BASE actual-unknown-bytes known-validation borrowed-lifetime");
    return 0;
}

static int digit(char c)
{
    return c >= '0' && c <= '9'   ? c - '0'
           : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                  : -1;
}

static int oracle(void)
{
    unsigned canonical, capacity, tamper;
    uint32_t initial;
    char hex[600];
    while (scanf("%u %u %" SCNu32 " %u %599s", &canonical, &capacity, &initial,
                 &tamper, hex) == 5)
    {
        unsigned char input[300], out[ADE_BYTES], tag[32];
        size_t n = !strcmp(hex, "-") ? 0 : strlen(hex) / 2;
        for (size_t i = 0; i < n; i++)
        {
            int a = digit(hex[2 * i]), b = digit(hex[2 * i + 1]);
            if (a < 0 || b < 0)
                return 2;
            input[i] = (unsigned char)(a * 16 + b);
        }
        struct ade_record record = sentinel(), saved = record;
        memset(out, 0xa5, sizeof out);
        memset(tag, 0xa5, sizeof tag);
        size_t used = 999;
        struct ade_service service = {initial, 7};
        int read = ade_read(input, n, &record), write = ADE_INVALID,
            auth = ADE_INVALID, apply = ADE_INVALID;
        if (read == ADE_OK)
        {
            write = ade_write(&record, (int)canonical, out, capacity, &used);
            auth = ade_tag(&record, key, tag);
            if (auth == ADE_OK)
            {
                if (tamper)
                    tag[31] ^= 1;
                apply = ade_apply(&record, key, tag, &service);
            }
        }
        printf("%d %d %d %zu %d %d %" PRIu32 " %" PRIu32 " ", read,
               read ? same(record, saved) : 1, write, used, auth, apply,
               service.total, service.privileged);
        for (size_t i = 0; i < sizeof out; i++)
        {
            printf("%02x", out[i]);
        }
        putchar(' ');
        for (unsigned i = 0; i < 32; i++)
        {
            printf("%02x", tag[i]);
        }
        putchar('\n');
    }
    return ferror(stdin) ? 1 : 0;
}

static int metrics(void)
{
    struct ade_record r = {0};
    r.count = 8;
    for (unsigned i = 0; i < 8; i++)
    {
        r.fields[i].tag = 3 + 7 - i;
        r.fields[i].length = 16;
        for (unsigned j = 0; j < 16; j++)
            r.fields[i].bytes[j] = (unsigned char)(i + j);
    }
    double relay[7], auth[7];
    unsigned char out[ADE_BYTES], tag[32];
    size_t used = 0;
    const unsigned iterations = 1000;
    uint64_t checksum = 0;
    for (unsigned s = 0; s < 7; s++)
    {
        clock_t start = clock();
        checksum = 0;
        for (unsigned i = 0; i < iterations; i++)
        {
            CHECK(ade_write(&r, 1, out, sizeof out, &used) == ADE_OK);
            checksum += used;
        }
        relay[s] = (double)(clock() - start) / CLOCKS_PER_SEC;
        CHECK(checksum == 145000);
        start = clock();
        checksum = 0;
        for (unsigned i = 0; i < iterations; i++)
        {
            CHECK(ade_tag(&r, key, tag) == ADE_OK);
            checksum += tag[0];
        }
        auth[s] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf("{\"iterations\":%u,\"bytes\":%zu,\"checksum\":%" PRIu64
           ",\"record_size\":%zu,\"record_alignment\":%zu,\"relay_seconds\":[",
           iterations, used, checksum, sizeof r, _Alignof(struct ade_record));
    for (unsigned i = 0; i < 7; i++)
    {
        printf("%s%.9f", i ? "," : "", relay[i]);
    }
    printf("],\"authentication_seconds\":[");
    for (unsigned i = 0; i < 7; i++)
    {
        printf("%s%.9f", i ? "," : "", auth[i]);
    }
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    for (unsigned i = 0; i < 32; i++)
        key[i] = (unsigned char)i;
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "evolution_records"))
        return records();
    if (!strcmp(argv[1], "evolution_auth"))
        return authentication();
    if (!strcmp(argv[1], "evolution_policy"))
        return policy();
    if (!strcmp(argv[1], "evolution_base"))
        return baseline();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
