#include "rational_units.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "ADO_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int fractional(void)
{
    struct adq_result r = {99, 77};
    CHECK(adq_scale(5, 3, 2, &r) == ADQ_OK && r.value == 7 && r.inexact == 1);
    CHECK(adq_scale(-5, 3, 2, &r) == ADQ_OK && r.value == -7 && r.inexact == 1);
    CHECK(adq_scale(6, 3, 2, &r) == ADQ_OK && r.value == 9 && r.inexact == 0);
    CHECK(adq_scale(-1, 1, 3, &r) == ADQ_OK && r.value == 0 && r.inexact == 1);
    CHECK(adq_scale(INT64_MAX, UINT32_MAX, UINT32_MAX, &r) == ADQ_OK &&
          r.value == INT64_MAX && !r.inexact);
    puts("ADQ_FRACTION no-wide-integer retained-remainder toward-zero");
    return 0;
}

static int boundary(void)
{
    struct adq_result r = {99, 77};
    struct adq_period p = {3, 2};
    CHECK(adq_scale(INT64_MIN, 1, 1, &r) == ADQ_OK && r.value == INT64_MIN &&
          !r.inexact);
    CHECK(adq_to_ticks((struct adq_microseconds){5}, p, &r) == ADQ_OK &&
          r.value == 3 && r.inexact);
    CHECK(adq_to_microseconds((struct adq_ticks){3}, p, &r) == ADQ_OK &&
          r.value == 4 && r.inexact);
    r = (struct adq_result){99, 77};
    CHECK(adq_scale(INT64_MAX, 2, 1, &r) == ADQ_RANGE && r.value == 99 &&
          r.inexact == 77);
    CHECK(adq_scale(INT64_MIN, 2, 1, &r) == ADQ_RANGE && r.value == 99 &&
          r.inexact == 77);
    CHECK(adq_scale(INT64_MAX, UINT32_MAX, UINT32_MAX - 1, &r) == ADQ_RANGE &&
          r.value == 99);
    CHECK(adq_scale(0, 0, 1, &r) == ADQ_INVALID && r.value == 99);
    CHECK(adq_scale(0, 1, 0, &r) == ADQ_INVALID && r.value == 99);
    CHECK(adq_scale(0, 1, 1, NULL) == ADQ_INVALID);
    puts("ADQ_BOUNDARY signed-minimum output-atomicity dimensional-direction");
    return 0;
}

static int base(void)
{
    struct ad_millis r = {77};
    CHECK(ad_milliseconds((struct ad_seconds){42}, &r) && r.value == 42000);
    CHECK(ad_milliseconds((struct ad_seconds){UINT32_MAX / 1000}, &r) &&
          r.value == 4294967000u);
    r.value = 77;
    CHECK(!ad_milliseconds((struct ad_seconds){UINT32_MAX / 1000 + 1}, &r) &&
          r.value == 77);
    puts("ADQ_BASE actual-seconds-provider quotient-boundary");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "rational_fraction"))
    {
        return fractional();
    }
    if (!strcmp(argv[1], "rational_boundary"))
    {
        return boundary();
    }
    if (!strcmp(argv[1], "rational_base"))
    {
        return base();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        int64_t v;
        uint32_t n, d;
        unsigned direction;
        while (scanf("%u %" SCNd64 " %" SCNu32 " %" SCNu32, &direction, &v, &n,
                     &d) == 4)
        {
            struct adq_result r = {99, 77};
            enum adq_status s;
            if (direction == 0)
            {
                s = adq_scale(v, n, d, &r);
            }
            else if (direction == 1)
            {
                s = adq_to_ticks((struct adq_microseconds){v},
                                 (struct adq_period){n, d}, &r);
            }
            else
            {
                s = adq_to_microseconds((struct adq_ticks){v},
                                        (struct adq_period){n, d}, &r);
            }
            printf("%u %" PRId64 " %u\n", (unsigned)s, r.value, r.inexact);
        }
        return 0;
    }
    if (!strcmp(argv[1], "metrics"))
    {
        double raw[7], wrapped[7];
        uint64_t checksum = 0;
        for (unsigned k = 0; k < 7; ++k)
        {
            for (unsigned mode = 0; mode < 2; ++mode)
            {
                clock_t begin = clock();
                uint64_t current = 0;
                for (unsigned i = 0; i < 100000; ++i)
                {
                    struct adq_result r;
                    enum adq_status s =
                        mode
                            ? adq_to_ticks(
                                  (struct adq_microseconds){(int64_t)i - 50000},
                                  (struct adq_period){3, 2}, &r)
                            : adq_scale((int64_t)i - 50000, 2, 3, &r);
                    CHECK(s == ADQ_OK);
                    current += (uint64_t)r.value + r.inexact;
                }
                if (k || mode)
                {
                    CHECK(checksum == current);
                }
                else
                {
                    checksum = current;
                }
                double seconds = (double)(clock() - begin) / CLOCKS_PER_SEC;
                if (mode)
                {
                    wrapped[k] = seconds;
                }
                else
                {
                    raw[k] = seconds;
                }
            }
        }
        printf(
            "{\"microseconds_size\":%zu,\"ticks_size\":%zu,\"result_size\":%zu,"
            "\"iterations\":100000,\"checksum\":%" PRIu64 ",\"raw\":[",
            sizeof(struct adq_microseconds), sizeof(struct adq_ticks),
            sizeof(struct adq_result), checksum);
        for (unsigned k = 0; k < 7; ++k)
        {
            printf("%s%.9f", k ? "," : "", raw[k]);
        }
        printf("],\"wrapped\":[");
        for (unsigned k = 0; k < 7; ++k)
        {
            printf("%s%.9f", k ? "," : "", wrapped[k]);
        }
        puts("]}");
        return 0;
    }
    return 2;
}
