#include "logging.h"
#include "logging_macros.h"
#include "metaprog.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            fprintf(stderr, "MPO_CHECK %d: %s\n", __LINE__, #x); \
            return 1;                                            \
        }                                                        \
    } while (0)

static struct mpl_field tick(unsigned *calls, uint32_t n)
{
    ++*calls;
    return mpl_number(n);
}

static struct mpl_sink *sink_tick(unsigned *calls, struct mpl_sink *s)
{
    ++*calls;
    return s;
}

static int evaluation(void)
{
    struct mpl_sink s = {0};
    unsigned c[4] = {0}, sc = 0;
    size_t i;
    CHECK(MPL_LOG(sink_tick(&sc, &s), tick(&c[0], 7)) == MPL_OK && c[0] == 1 &&
          sc == 1);
    CHECK(MPL_LOG(&s, tick(&c[0], 8), tick(&c[1], 9)) == MPL_OK);
    CHECK(MPL_LOG(&s, tick(&c[0], 10), tick(&c[1], 11), tick(&c[2], 12)) ==
          MPL_OK);
    CHECK(MPL_LOG(&s, tick(&c[0], 13), tick(&c[1], 14), tick(&c[2], 15),
                  tick(&c[3], 16)) == MPL_OK);
    CHECK(c[0] == 4 && c[1] == 3 && c[2] == 2 && c[3] == 1);
    for (i = 0; i < 4; ++i)
    {
        CHECK(s.records[i].count == i + 1);
    }
    CHECK(s.records[3].fields[0].number == 13 &&
          s.records[3].fields[3].number == 16);
    CHECK(MPL_LOG(sink_tick(&sc, &s), tick(&c[0], 17)) == MPL_FULL && sc == 2 &&
          c[0] == 5);
    s.count = 0;
    CHECK(MPL_LOG(&s, ((struct mpl_field){MPL_NUMBER, 19, NULL, 0})) ==
              MPL_OK &&
          s.records[0].fields[0].number == 19);
    puts("MPL_EVAL finite-arity once separate-effects protected-comma");
    return 0;
}

static int copying(void)
{
    struct mpl_sink s = {0};
    unsigned char bytes[] = {0, 255, 2, 3, 4, 5, 6, 7};
    char file[] = "synthetic.c";
    struct mpl_site site = {file, 77};
    struct mpl_field fields[] = {mpl_bytes(bytes, 8), mpl_number(UINT32_MAX)};
    unsigned line;
    CHECK(mpl_write(&s, &site, fields, 2) == MPL_OK);
    bytes[7] = 99;
    file[0] = 'X';
    CHECK(s.records[0].fields[0].bytes[7] == 7 &&
          s.records[0].fields[0].bytes[1] == 255 &&
          s.records[0].fields[1].number == UINT32_MAX);
    CHECK(s.records[0].has_site == 1 && s.records[0].line == 77 &&
          !strcmp(s.records[0].file, "synthetic.c"));
    line = __LINE__ + 1;
    CHECK(MPL_LOG_AT(&s, mpl_bytes(NULL, 0)) == MPL_OK);
    CHECK(s.records[1].line == line && !strcmp(s.records[1].file, __FILE__) &&
          s.records[1].fields[0].length == 0);
    line = __LINE__ + 1;
    CHECK(MPL_LOG_AT0(&s) == MPL_OK);
    CHECK(s.records[2].line == line && s.records[2].has_site &&
          s.records[2].count == 0);
    CHECK(MPL_LOG(&s, mpl_bytes((const unsigned char *)"%n", 2)) == MPL_OK &&
          s.records[3].fields[0].bytes[0] == '%' &&
          s.records[3].fields[0].bytes[1] == 'n');
    puts("MPL_COPY inline-binary filename line no-retained-borrow");
    return 0;
}

static int atomicity(void)
{
    struct mpl_sink s = {0}, before;
    struct mpl_field f[] = {mpl_number(1), {7, 0, NULL, 0}};
    char file[256];
    struct mpl_site p = {file, 1};
    unsigned char data[9] = {0};
    unsigned c = 0;
    CHECK(MPL_LOG0(&s) == MPL_OK && s.count == 1 && s.records[0].count == 0 &&
          !s.records[0].has_site);
    before = s;
    CHECK(mpl_write(&s, NULL, f, 2) == MPL_INVALID &&
          !memcmp(&s, &before, sizeof s));
    f[1] = mpl_bytes(data, 9);
    CHECK(mpl_write(&s, NULL, f, 2) == MPL_RANGE &&
          !memcmp(&s, &before, sizeof s));
    f[1] = mpl_bytes(NULL, 1);
    CHECK(mpl_write(&s, NULL, f, 2) == MPL_INVALID &&
          !memcmp(&s, &before, sizeof s));
    memset(file, 'a', sizeof file);
    CHECK(mpl_write(&s, &p, NULL, 0) == MPL_RANGE &&
          !memcmp(&s, &before, sizeof s));
    file[255] = 0;
    CHECK(mpl_write(&s, &p, NULL, 0) == MPL_OK &&
          s.records[1].file[254] == 'a');
    before = s;
    p.line = 0;
    CHECK(mpl_log1(&s, p, tick(&c, 8)) == MPL_INVALID && c == 1 &&
          !memcmp(&s, &before, sizeof s));
    CHECK(mpl_write(&s, NULL, NULL, 1) == MPL_INVALID &&
          !memcmp(&s, &before, sizeof s));
    CHECK(mpl_write(NULL, NULL, NULL, 0) == MPL_INVALID);
    s.count = 4;
    before = s;
    CHECK(MPL_LOG0(&s) == MPL_FULL && !memcmp(&s, &before, sizeof s));
    puts(
        "MPL_ATOMIC explicit-zero unchanged-refusal evaluated-effects-retained");
    return 0;
}

static int baseline(void)
{
    unsigned n, calls = 91;
    uint32_t out = 92;
    for (n = 0; n <= 4; ++n)
    {
        CHECK(mp_variadic(n, 7, &calls, &out) && calls == n &&
              out == n * 7 + n * (n - 1) / 2);
    }
    CHECK(!mp_variadic(5, 7, &calls, &out) && calls == 4 && out == 34);
    CHECK(mp_variadic(4, UINT32_MAX, &calls, &out) && calls == 4 && out == 2);
    puts("MPL_BASE actual-numeric-dispatch modulo empty-identity");
    return 0;
}

static uint64_t fold(uint64_t h, uint64_t n)
{
    return h * 257 + n;
}

static uint64_t digest(const struct mpl_record *r)
{
    uint64_t h = 0;
    size_t i, j;
    h = fold(h, r->has_site);
    h = fold(h, r->line);
    h = fold(h, r->count);
    for (i = 0; r->file[i]; ++i)
    {
        h = fold(h, (unsigned char)r->file[i]);
    }
    for (i = 0; i < r->count; ++i)
    {
        const struct mpl_value *v = &r->fields[i];
        h = fold(h, v->kind);
        h = fold(h, v->number);
        h = fold(h, v->length);
        for (j = 0; j < v->length; ++j)
        {
            h = fold(h, v->bytes[j]);
        }
    }
    return h;
}

static int driver(void)
{
    unsigned used, mode, sl, n;
    uint32_t seed;
    while (scanf("%u%u%u%u%" SCNu32, &used, &mode, &sl, &n, &seed) == 5)
    {
        struct mpl_sink s = {0}, before;
        struct mpl_field fields[5];
        unsigned char data[5][9];
        char file[257];
        struct mpl_site p;
        size_t i, j;
        int status;
        if (used > 5 || mode > 3 || sl > 256 || n > 5)
        {
            return 2;
        }
        for (i = 0; i < n; ++i)
        {
            unsigned kind, len, isnull;
            if (scanf("%u%u%u", &kind, &len, &isnull) != 3 || len > 9)
            {
                return 2;
            }
            for (j = 0; j < 9; ++j)
            {
                data[i][j] = (unsigned char)(seed + i + j);
            }
            fields[i] = (struct mpl_field){kind, seed + (uint32_t)i,
                                           isnull ? NULL : data[i], len};
        }
        for (i = 0; i < sl; ++i)
        {
            file[i] = (char)('a' + i % 26);
        }
        file[sl] = 0;
        p = (struct mpl_site){mode == 2 ? NULL : file, mode == 3 ? 0 : 17};
        s.count = used;
        before = s;
        status = mpl_write(&s, mode ? &p : NULL, fields, n);
        printf("%d %zu %" PRIu64 " %d\n", status, s.count,
               status == MPL_OK ? digest(&s.records[used]) : UINT64_C(99),
               status == MPL_OK ? 0 : !memcmp(&s, &before, sizeof s));
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    double macro[7], direct[7];
    size_t sample, round;
    uint64_t sum = 0;
    struct mpl_sink s = {0};
    struct mpl_site p = {NULL, 0};
    for (sample = 0; sample < 7; ++sample)
    {
        clock_t start = clock();
        CHECK(start != (clock_t)-1);
        sum = 0;
        for (round = 0; round < 10000; ++round)
        {
            s.count = 0;
            CHECK(MPL_LOG(&s, mpl_number(1), mpl_number(2), mpl_number(3),
                          mpl_number(4)) == MPL_OK);
            sum += s.records[0].fields[3].number;
        }
        macro[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
        CHECK(sum == 40000);
        start = clock();
        CHECK(start != (clock_t)-1);
        sum = 0;
        for (round = 0; round < 10000; ++round)
        {
            s.count = 0;
            CHECK(mpl_log4(&s, p, mpl_number(1), mpl_number(2), mpl_number(3),
                           mpl_number(4)) == MPL_OK);
            sum += s.records[0].fields[3].number;
        }
        direct[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
        CHECK(sum == 40000);
    }
    printf(
        "{\"iterations\":10000,\"checksum\":40000,\"sink_size\":%zu,\"record_size\":%zu,\"field_size\":%zu,\"macro_seconds\":[",
        sizeof s, sizeof(struct mpl_record), sizeof(struct mpl_field));
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", macro[sample]);
    }
    printf("],\"direct_seconds\":[");
    for (sample = 0; sample < 7; ++sample)
    {
        printf("%s%.9f", sample ? "," : "", direct[sample]);
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
    if (!strcmp(argv[1], "logging_eval"))
    {
        return evaluation();
    }
    if (!strcmp(argv[1], "logging_copy"))
    {
        return copying();
    }
    if (!strcmp(argv[1], "logging_atomic"))
    {
        return atomicity();
    }
    if (!strcmp(argv[1], "logging_base"))
    {
        return baseline();
    }
    if (!strcmp(argv[1], "driver"))
    {
        return driver();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
