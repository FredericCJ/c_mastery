#include "conversion.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "CONVERSION-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                  \
        }                                                              \
    } while (0)

struct converter
{
    unsigned calls;
    int partial, range;
};

static struct lc_float convert(void *v, const char *s)
{
    struct converter *c = v;
    ++c->calls;
    char *end;
    errno = 0;
    double d = strtod(s, &end);
    return (struct lc_float){d, c->partial ? 0 : (size_t)(end - s),
                             c->range || errno == ERANGE};
}

int main(void)
{
    const char *signeds[] = {"0",  "-12",  "12x",  "",    "999",  "+1", " -1",
                             "1 ", "+-12", "-128", "127", "-129", "128"};
    const int values[] = {0, -12, 0, 0, 0, 1, 0, 0, 0, -128, 127, 0, 0};

    const enum lc_status statuses[] = {LC_OK,     LC_OK, LC_SYNTAX, LC_SYNTAX,
                                       LC_RANGE,  LC_OK, LC_SYNTAX, LC_SYNTAX,
                                       LC_SYNTAX, LC_OK, LC_OK,     LC_RANGE,
                                       LC_RANGE};

    for (unsigned i = 0; i < 13; ++i)
    {
        int32_t out = 77;
        CHECK(lc_i8((const unsigned char *)signeds[i], strlen(signeds[i]), 1,
                    &out) == statuses[i]);
        CHECK(out == (statuses[i] == LC_OK ? values[i] : 77));
    }
    int32_t result = 77;
    CHECK(lc_i8((const unsigned char *)"+1", 2, 0, &result) == LC_SYNTAX &&
          result == 77);
    const char *unsigneds[] = {"-1", "+1",   "010", "0x10", "12x", "   ",
                               "08", "0b10", "0Xf", "0x",   "+"};

    const enum lc_status dec[] = {LC_SYNTAX, LC_OK,     LC_OK,    LC_SYNTAX,
                                  LC_SYNTAX, LC_SYNTAX, LC_OK,    LC_SYNTAX,
                                  LC_SYNTAX, LC_SYNTAX, LC_SYNTAX},
                         base[] = {LC_SYNTAX, LC_OK,     LC_OK,     LC_OK,
                                   LC_SYNTAX, LC_SYNTAX, LC_SYNTAX, LC_SYNTAX,
                                   LC_OK,     LC_SYNTAX, LC_SYNTAX};

    const uintmax_t decimal[] = {0, 1, 10, 0, 0, 0, 8},
                    prefixed[] = {0, 1, 8, 16, 0, 0, 0, 0, 15};
    for (unsigned i = 0; i < 11; ++i)
        for (int mode = 0; mode < 2; ++mode)
        {
            uintmax_t value = 77;
            errno = ERANGE;
            enum lc_status status =
                lc_unsigned((const unsigned char *)unsigneds[i],
                            strlen(unsigneds[i]), mode, UINTMAX_MAX, &value);
            CHECK(status == (mode ? base[i] : dec[i]));
            if (status == LC_OK)
                CHECK(value == (mode ? prefixed[i] : decimal[i]));
            else
                CHECK(value == 77);
        }
    char maximum[128], overflow[129];
    int count = snprintf(maximum, sizeof maximum, "%" PRIuMAX, UINTMAX_MAX);
    CHECK(count > 0 && count < 126);
    memcpy(overflow, maximum, (size_t)count);
    overflow[count] = '0';
    overflow[count + 1] = 0;
    uintmax_t u = 77;
    CHECK(lc_unsigned((unsigned char *)maximum, (size_t)count, 0, UINTMAX_MAX,
                      &u) == LC_OK &&
          u == UINTMAX_MAX);
    u = 77;
    CHECK(lc_unsigned((unsigned char *)overflow, (size_t)count + 1, 0,
                      UINTMAX_MAX, &u) == LC_RANGE &&
          u == 77);
    CHECK(lc_unsigned((unsigned char *)"256", 3, 0, 255, &u) == LC_RANGE &&
          u == 77);
    const char *good[] = {"0", "1.5e0", "-1e2", "1e-3", "9.01e12"};
    const double expected[] = {0, 1.5, -100, 0.001, 9.01e12};
    struct converter c = {0};
    for (unsigned i = 0; i < 5; ++i)
    {
        double out = 77;
        CHECK(lc_wire_float((const unsigned char *)good[i], strlen(good[i]),
                            convert, &c, &out) == LC_OK &&
              out == expected[i]);
    }
    const char *bad[] = {"1.5",   " 1.5e0", "+1e0", "inf",  "nan",
                         "0x1p0", "1.0e0",  "01e0", "1e+0", "1e-0",
                         "0e0",   "-0",     "1.e0", "1e01", "1e0x"};
    for (unsigned i = 0; i < 15; ++i)
    {
        double out = 77;
        unsigned calls = c.calls;
        CHECK(lc_wire_float((const unsigned char *)bad[i], strlen(bad[i]),
                            convert, &c, &out) == LC_SYNTAX &&
              out == 77 && c.calls == calls);
    }
    double out = 77;
    c.partial = 1;
    CHECK(lc_wire_float((const unsigned char *)"1e0", 3, convert, &c, &out) ==
              LC_BACKEND &&
          out == 77);
    c.partial = 0;
    c.range = 1;
    CHECK(lc_wire_float((const unsigned char *)"1e0", 3, convert, &c, &out) ==
              LC_RANGE &&
          out == 77);
    c.range = 0;
    CHECK(lc_wire_float((const unsigned char *)"1e9999", 6, convert, &c,
                        &out) == LC_RANGE &&
          out == 77);
    CHECK(lc_wire_float((const unsigned char *)"1e-9999", 7, convert, &c,
                        &out) == LC_RANGE &&
          out == 77);
    puts(
        "CONVERSION-PASS:grammar:errno:range:canonical-wire:failure-publication");
    return 0;
}
