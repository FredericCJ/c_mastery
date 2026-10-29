#include "performance.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "PE_ASSERT:%u:%s\n", __LINE__, #x); \
            return 41;                                          \
        }                                                       \
    } while (0)

static int equation(void)
{
    struct pe_equation r, old;
    CHECK(!pe_equation(1200000, (struct pe_ratio){3, 2}, 2000000000,
                       (struct pe_ratio){3, 5}, (struct pe_ratio){3, 1}, 0,
                       &r));
    CHECK(r.baseline_ns.n == 900000 && r.baseline_ns.d == 1 &&
          r.new_ns.n == 540000 && r.new_ns.d == 1);
    CHECK(r.speedup.n == 5 && r.speedup.d == 3 && r.infinite_limit.n == 5 &&
          r.infinite_limit.d == 2);
    uint64_t costs[] = {0, 359999, 360000, 360001, 500000};
    for (unsigned i = 0; i < 5; ++i)
    {
        CHECK(!pe_equation(1200000, (struct pe_ratio){3, 2}, 2000000000,
                           (struct pe_ratio){3, 5}, (struct pe_ratio){3, 1},
                           costs[i], &r));
        CHECK(r.new_ns.n == 540000 + costs[i] && r.new_ns.d == 1);
        printf("EQUATION %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64
               " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
               costs[i], r.baseline_ns.n, r.new_ns.n, r.speedup.n, r.speedup.d,
               r.infinite_limit.n, r.infinite_limit.d);
    }
    CHECK(!pe_equation(1, (struct pe_ratio){1, 3}, 7, (struct pe_ratio){1, 7},
                       (struct pe_ratio){5, 2}, 0, &r));
    printf("FRACTION %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           r.baseline_ns.n, r.baseline_ns.d, r.new_ns.n, r.new_ns.d);
    CHECK(!pe_equation(1, (struct pe_ratio){1, 1}, 1, (struct pe_ratio){1, 1},
                       (struct pe_ratio){1, 1}, 0, &r) &&
          r.unbounded_limit);
    memset(&r, 0xa5, sizeof r);
    old = r;
    CHECK(pe_equation(UINT64_MAX, (struct pe_ratio){UINT64_MAX, 1}, 1,
                      (struct pe_ratio){1, 2}, (struct pe_ratio){2, 1}, 0,
                      &r) &&
          !memcmp(&r, &old, sizeof r));
    CHECK(pe_equation(1, (struct pe_ratio){1, 0}, 1, (struct pe_ratio){1, 2},
                      (struct pe_ratio){2, 1}, 0, &r));
    CHECK(pe_equation(1, (struct pe_ratio){1, 1}, 1, (struct pe_ratio){3, 2},
                      (struct pe_ratio){2, 1}, 0, &r));
    return 0;
}

static int stats(void)
{
    uint64_t c[] = {100, 104, 96, 100, 110}, a[] = {95, 93, 97, 95, 120};
    struct pe_stats s;
    CHECK(!pe_stats(c, 5, &s) && s.minimum == 96 && s.maximum == 110 &&
          s.median_floor == 100 && !s.median_half);
    CHECK(!pe_stats(a, 5, &s) && s.minimum == 93 && s.maximum == 120 &&
          s.median_floor == 95 && !s.median_half);
    uint64_t top[] = {UINT64_MAX, UINT64_MAX};
    CHECK(!pe_stats(top, 2, &s) && s.median_floor == UINT64_MAX &&
          !s.median_half);
    uint64_t odd[] = {2, 3};
    CHECK(!pe_stats(odd, 2, &s) && s.median_floor == 2 && s.median_half);
    struct pe_stats old = s;
    CHECK(pe_stats(a, 0, &s) && !memcmp(&s, &old, sizeof s));
    puts("STATS 100 95 96 110 93 120 2 1");
    return 0;
}

static int equal(void)
{
    uint32_t a[1024], expected, c, m;
    uint64_t checks = 0;
    for (uint32_t n = 0; n <= 1024; ++n)
    {
        for (uint32_t i = 0; i < n; ++i)
        {
            a[i] = (i / 2) * 8388608u;
        }
        uint32_t keys[] = {0, 7, 0x80000000u, 0xfffffffdu, UINT32_MAX};
        for (unsigned j = 0; j < 5; ++j)
        {
            expected = 0;
            while (expected < n && a[expected] < keys[j])
            {
                ++expected;
            }
            c = m = UINT32_MAX;
            CHECK(pe_c(a, n, keys[j], &c) == ALGO_OK &&
                  pe_asm(a, n, keys[j], &m) == ALGO_OK && c == expected &&
                  m == expected);
            ++checks;
        }
    }
    uint32_t bad[] = {2, 1};
    c = m = 17;
    CHECK(pe_c(bad, 2, 1, &c) == ALGO_BAD &&
          pe_asm(bad, 2, 1, &m) == ALGO_BAD && c == 17 && m == 17);
    CHECK(pe_c(a, 1025, 1, &c) == ALGO_BAD &&
          pe_asm(a, 1025, 1, &m) == ALGO_BAD);
    CHECK(pe_c(0, 1, 1, &c) == ALGO_BAD && pe_asm(0, 1, 1, &m) == ALGO_BAD);
    printf("EQUAL %" PRIu64 "\n", checks);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    int r;
    if (!strcmp(argv[1], "equation"))
    {
        r = equation();
    }
    else if (!strcmp(argv[1], "stats"))
    {
        r = stats();
    }
    else if (!strcmp(argv[1], "equal"))
    {
        r = equal();
    }
    else
    {
        return 2;
    }
    if (!r)
    {
        printf("OBS %s\n", argv[1]);
    }
    return r;
}
