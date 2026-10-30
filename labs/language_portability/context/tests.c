#include "context.h"
#include <errno.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "LP_CHECK %d %s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

/* Field-wise equality: the decoder state must be bytewise unchanged after a
   refused feed, and struct padding is not compared. */
static int same(const struct lc_utf *a, const struct lc_utf *b)
{
    return a->scalar == b->scalar && a->minimum == b->minimum &&
           a->remaining == b->remaining && a->bytes == b->bytes &&
           a->scalars == b->scalars;
}

static void strict(void)
{
    const unsigned char bad[3][4] = {
        {0xc0, 0x80, 0, 0}, {0xed, 0xa0, 0x80, 0}, {0xf4, 0x90, 0x80, 0x80}};
    size_t lengths[] = {2, 3, 4};
    for (size_t i = 0; i < 3; ++i)
    {
        struct lc_utf s;
        lc_utf_init(&s);
        struct lc_utf saved = s;
        CHECK(lc_utf_feed(&s, bad[i], lengths[i]) == LC_ENCODING &&
              same(&s, &saved));
    }
    struct lc_utf s;
    lc_utf_init(&s);
    CHECK(lc_utf_feed(&s, (const unsigned char *)"A", 1) == LC_OK);
    size_t out = 77;
    CHECK(lc_utf_finish(&s, &out) == LC_OK && out == 1);
    CHECK(lc_utf_feed(&s, NULL, 0) == LC_OK);
    CHECK(lc_utf_feed(&s, NULL, 1) == LC_INVALID);
    unsigned char bytes[64];
    memset(bytes, 'A', sizeof bytes);
    lc_utf_init(&s);
    CHECK(lc_utf_feed(&s, bytes, 64) == LC_OK);
    struct lc_utf saved = s;
    CHECK(lc_utf_feed(&s, bytes, 1) == LC_RANGE && same(&s, &saved));
}

static void partial(void)
{
    const unsigned char euro[] = {0xe2, 0x82, 0xac};
    struct lc_utf a, b;
    lc_utf_init(&a);
    lc_utf_init(&b);
    CHECK(lc_utf_feed(&a, euro, 1) == LC_OK);
    size_t out = 77;
    CHECK(lc_utf_finish(&a, &out) == LC_INCOMPLETE && out == 77);
    CHECK(lc_utf_feed(&b, (const unsigned char *)"A", 1) == LC_OK);
    CHECK(lc_utf_feed(&a, euro + 1, 2) == LC_OK);
    CHECK(lc_utf_finish(&a, &out) == LC_OK && out == 1);
    CHECK(lc_utf_finish(&b, &out) == LC_OK && out == 1);
}

static void locale_case(void)
{
    struct lc_locale a, b;
    wchar_t value = 77;
    size_t used = 77;
    CHECK(setlocale(LC_CTYPE, "C"));
    lc_locale_init(&a);
    CHECK(lc_locale_one(&a, "A", 1, &value, &used) == LC_OK && value == L'A' &&
          used == 1);
    value = 77;
    used = 77;
    errno = 0;
    CHECK(lc_locale_one(&a, "\xc3\xa9", 2, &value, &used) == LC_ENCODING &&
          errno == EILSEQ && value == 77 && used == 77);
    CHECK(lc_locale_one(&a, "A", 1, &value, &used) == LC_POISONED);
    lc_locale_init(&a);
    CHECK(lc_locale_one(&a, "", 1, &value, &used) == LC_OK && value == 0 &&
          used == 0 && mbsinit(&a.state));
    const char *locale = setlocale(LC_CTYPE, "C.UTF-8");
    CHECK(locale);
    lc_locale_init(&a);
    lc_locale_init(&b);
    value = 77;
    used = 77;
    CHECK(lc_locale_one(&a, "\xc3", 1, &value, &used) == LC_INCOMPLETE &&
          value == 77 && used == 77);
    CHECK(lc_locale_one(&b, "\xe2\x82", 2, &value, &used) == LC_INCOMPLETE);
    CHECK(lc_locale_one(&a, "\xa9", 1, &value, &used) == LC_OK &&
          value == 0xe9 && used == 1);
    CHECK(lc_locale_one(&b, "\xac", 1, &value, &used) == LC_OK &&
          value == 0x20ac && used == 1);
    CHECK(iswalpha(L'A') and not iswalpha(L'1'));
    printf("LC_LOCALE %s wchar_bits%zu independent_states2\n", locale,
           sizeof(wchar_t) * CHAR_BIT);
    CHECK(setlocale(LC_CTYPE, "C"));
}

static void combining(void)
{
    const unsigned char one[] = {0xc3, 0xa9}, two[] = {0x65, 0xcc, 0x81};
    struct lc_utf a, b;
    lc_utf_init(&a);
    lc_utf_init(&b);
    size_t x = 77, y = 77;
    CHECK(lc_utf_feed(&a, one, 2) == LC_OK && lc_utf_feed(&b, two, 3) == LC_OK);
    CHECK(lc_utf_finish(&a, &x) == LC_OK && lc_utf_finish(&b, &y) == LC_OK &&
          x == 1 && y == 2);
}

enum
{
    ORACLE_MAX_BYTES = 8,
    NOT_SET = 77
};

/* Read "n split byte..." records (bytes in hex) and print the status and the
   scalar count after feeding the bytes in two pieces at offset split. */
static void oracle(void)
{
    size_t n;
    size_t split;
    while (scanf("%zu %zu", &n, &split) == 2)
    {
        CHECK(n <= ORACLE_MAX_BYTES && split <= n);
        unsigned char bytes[ORACLE_MAX_BYTES];
        for (size_t i = 0; i < n; ++i)
        {
            unsigned v;
            CHECK(scanf("%x", &v) == 1 && v <= 255);
            bytes[i] = (unsigned char)v;
        }
        struct lc_utf s;
        lc_utf_init(&s);
        int status = lc_utf_feed(&s, bytes, split);
        if (status == LC_OK)
        {
            status = lc_utf_feed(&s, bytes + split, n - split);
        }
        size_t out = NOT_SET;
        if (status == LC_OK)
        {
            status = lc_utf_finish(&s, &out);
        }
        printf("%d %zu\n", status, out);
    }
}

static void metrics(void)
{
    printf(
        "{\"utf_bytes\":%zu,\"locale_bytes\":%zu,\"byte_limit\":64,\"wchar_bits\":%zu}\n",
        sizeof(struct lc_utf), sizeof(struct lc_locale),
        sizeof(wchar_t) * CHAR_BIT);
}

struct lp_case
{
    const char *name;
    void (*run)(void);
    int prints_verdict; /* 1: print "LP <name> validated" after success */
};

static const struct lp_case cases[] = {
    {"oracle", oracle, 0},      {"metrics", metrics, 0},
    {"strict", strict, 1},      {"partial", partial, 1},
    {"locale", locale_case, 1}, {"combining", combining, 1}};

/* Exit status: 0 pass, 1 failed check (CHECK exits). */
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            cases[i].run();
            if (cases[i].prints_verdict)
            {
                printf("LP %s validated\n", argv[1]);
            }
            return 0;
        }
    }
    CHECK(0);
    return 1;
}
