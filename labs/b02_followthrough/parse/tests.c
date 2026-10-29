#include "parse.h"
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "BF_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static int baseline(void)
{
    const unsigned char *cases[] = {(const unsigned char *)"",
                                    (const unsigned char *)"0",
                                    (const unsigned char *)"-2147483648",
                                    (const unsigned char *)"2147483647",
                                    (const unsigned char *)"2147483648",
                                    (const unsigned char *)"-2147483649",
                                    (const unsigned char *)"12\0x"};
    size_t sizes[] = {0, 1, 11, 10, 10, 11, 4};
    int statuses[] = {B2_INVALID, B2_OK,    B2_OK,     B2_OK,
                      B2_RANGE,   B2_RANGE, B2_INVALID};
    for (unsigned i = 0; i < 7; i++)
    {
        int32_t a = 77, b = 77, c = 77;
        CHECK(b2_parse(cases[i], sizes[i], &a) == statuses[i]);
        errno = ERANGE;
        CHECK(bf_parse_libc(cases[i], sizes[i], 10, &b) == statuses[i]);
        CHECK(bf_parse_radix(cases[i], sizes[i], 10, &c) == statuses[i]);
        CHECK(a == b && b == c);
    }
    puts("BF baseline validated actual-B2-and-hosted-adapter");
    return 0;
}

static int grammar(void)
{
    const char *bad[] = {"-0", "00",   "01", "+1", " 1",
                         "1x", "0x10", "a",  "--1"};
    for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; i++)
    {
        int32_t out = 77;
        CHECK(bf_parse_radix((const unsigned char *)bad[i], strlen(bad[i]), 16,
                             &out) == B2_INVALID &&
              out == 77);
    }
    int32_t out = 77;
    CHECK(bf_parse_radix((const unsigned char *)"Z", 1, 36, &out) == B2_OK &&
          out == 35);
    CHECK(bf_parse_radix((const unsigned char *)"Z", 1, 35, &out) ==
              B2_INVALID &&
          out == 35);
    puts("BF grammar validated canonical-uppercase-no-prefix-no-negative-zero");
    return 0;
}

static int range_case(void)
{
    unsigned char huge[1001];
    for (unsigned i = 0; i < 1000; i++)
    {
        huge[i] = 57;
    }
    huge[1000] = 120;
    int32_t out = 77;
    CHECK(bf_parse_radix(huge, 1000, 10, &out) == B2_RANGE && out == 77);
    CHECK(bf_parse_radix(huge, 1001, 10, &out) == B2_INVALID && out == 77);
    struct bf_parse s = {0};
    CHECK(bf_parse_reset(&s, 10) == B2_OK);
    CHECK(bf_parse_feed(&s, huge, 1000) == B2_OK);
    CHECK(bf_parse_feed(&s, huge + 1000, 1) == B2_OK);
    CHECK(bf_parse_finish(&s, &out) == B2_INVALID && out == 77);
    puts("BF range validated complete-grammar-before-overflow");
    return 0;
}

static int lifecycle(void)
{
    struct bf_parse s = {0};
    int32_t out = 77;
    CHECK(bf_parse_reset(&s, 10) == B2_OK);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"x", 1) == B2_OK);
    CHECK(bf_parse_finish(&s, &out) == B2_INVALID && out == 77);
    CHECK(bf_parse_reset(&s, 10) == B2_OK);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"12", 2) == B2_OK &&
          out == 77);
    CHECK(bf_parse_reset(&s, 1) == B2_INVALID && s.magnitude == 12 &&
          s.base == 10 && s.phase == 0);
    CHECK(bf_parse_feed(&s, NULL, 1) == B2_INVALID && s.magnitude == 12 &&
          s.phase == 0);
    CHECK(bf_parse_finish(&s, NULL) == B2_INVALID && s.phase == 0);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"3", 1) == B2_OK);
    CHECK(bf_parse_finish(&s, &out) == B2_OK && out == 123);
    CHECK(bf_parse_feed(&s, NULL, 0) == BF_CLOSED);
    CHECK(bf_parse_finish(&s, &out) == B2_OK && out == 123);
    CHECK(bf_parse_reset(&s, 2) == B2_OK);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"-", 1) == B2_OK);
    CHECK(bf_parse_cancel(&s) == B2_OK);
    CHECK(bf_parse_finish(&s, &out) == BF_CANCELLED && out == 123);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"1", 1) == BF_CLOSED);
    CHECK(bf_parse_reset(&s, 2) == B2_OK);
    CHECK(bf_parse_feed(&s, (const unsigned char *)"-101", 4) == B2_OK);
    CHECK(bf_parse_finish(&s, &out) == B2_OK && out == -5);
    puts("BF lifecycle validated finish-reset-cancel-no-prefix-publication");
    return 0;
}

static int adapter(void)
{
    const char *locales[] = {"C", "C.UTF-8"};
    for (unsigned i = 0; i < 2; i++)
    {
        const char *loc = setlocale(LC_ALL, locales[i]);
        CHECK(loc);
        int32_t out = 77;
        errno = ERANGE;
        CHECK(bf_parse_libc((const unsigned char *)"0", 1, 10, &out) == B2_OK &&
              out == 0);
        CHECK(bf_parse_libc((const unsigned char *)"2147483648", 10, 10,
                            &out) == B2_RANGE &&
              out == 0);
        CHECK(bf_parse_libc((const unsigned char *)"12x", 3, 10, &out) ==
                  B2_INVALID &&
              out == 0);
    }
    CHECK(setlocale(LC_ALL, "C"));
    char *end;
    const char *text = "x";
    errno = 0;
    CHECK(strtol(text, &end, 10) == 0 && end == text);
    text = "0";
    CHECK(strtol(text, &end, 10) == 0 && end == text + 1);
    text = "12x";
    CHECK(strtol(text, &end, 10) == 12 && end == text + 2);
    printf("BF adapter validated errno-full-consumption LONG_BITS=%zu "
           "locales=C,C.UTF-8\n",
           sizeof(long) * CHAR_BIT);
    return 0;
}

static unsigned hex(char c)
{
    return c <= '9' ? (unsigned)(c - '0') : (unsigned)(c - 'a' + 10);
}

static int oracle(void)
{
    unsigned base, split;
    char text[2203];
    unsigned char bytes[1101];
    while (scanf("%u %u %2202s", &base, &split, text) == 3)
    {
        size_t n = !strcmp(text, "-") ? 0 : strlen(text) / 2;
        CHECK(n <= 1101);
        for (size_t i = 0; i < n; i++)
        {
            bytes[i] =
                (unsigned char)(16 * hex(text[2 * i]) + hex(text[2 * i + 1]));
        }
        int32_t a = 77, b = 77, c = 77, d = 77;
        int ra = base == 10 ? b2_parse(bytes, n, &a) : -1,
            rb = bf_parse_radix(bytes, n, base, &b);
        errno = ERANGE;
        int rc = bf_parse_libc(bytes, n, base, &c);
        struct bf_parse s = {0};
        CHECK(bf_parse_reset(&s, base) == B2_OK);
        if (split > n)
        {
            for (size_t i = 0; i < n; i++)
            {
                CHECK(bf_parse_feed(&s, bytes + i, 1) == B2_OK);
            }
        }
        else
        {
            CHECK(bf_parse_feed(&s, bytes, split) == B2_OK);
            CHECK(bf_parse_feed(&s, bytes + split, n - split) == B2_OK);
        }
        CHECK(d == 77);
        int rd = bf_parse_finish(&s, &d);
        printf("%d %ld %d %ld %d %ld %d %ld\n", ra, (long)a, rb, (long)b, rc,
               (long)c, rd, (long)d);
    }
    return 0;
}

int main(int n, char **v)
{
    if (n != 2)
    {
        return 2;
    }
    return !strcmp(v[1], "baseline")    ? baseline()
           : !strcmp(v[1], "grammar")   ? grammar()
           : !strcmp(v[1], "range")     ? range_case()
           : !strcmp(v[1], "lifecycle") ? lifecycle()
           : !strcmp(v[1], "adapter")   ? adapter()
           : !strcmp(v[1], "oracle")    ? oracle()
                                        : 2;
}
