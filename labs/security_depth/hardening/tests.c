#define _POSIX_C_SOURCE 200809L
#include "hardening.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "SH_ASSERT:%s\n", #x); \
            return 41;                             \
        }                                          \
    } while (0)
static unsigned char payload[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

static int valid(void)
{
    unsigned out = 77;
    CHECK(sh_copy(payload, 8, &out) == 0 && out == 36);
    for (size_t n = 0; n <= 8; ++n)
    {
        CHECK(sh_copy(payload, n, &out) == 0);
        CHECK(out == n * (n + 1) / 2);
    }
    out = 77;
    CHECK(sh_copy(payload, 9, &out) == -1 && out == 77);
    CHECK(sh_copy(NULL, 1, &out) == -1 && out == 77);
    CHECK(sh_copy(NULL, 0, &out) == 0 && out == 0);
    CHECK(sh_copy(payload, 1, NULL) == -1);
    CHECK(sh_flawed_copy(payload, 8) == 36 && sh_optimized_away(9) == 9);
    puts("OBS bounds defined repair preserves rejection output");
    return 0;
}

static int policy(void)
{
    unsigned n = 7;
    CHECK(sh_flawed_authorize(1, 0, &n) == 0 && n == 8);
    n = 7;
    CHECK(sh_authorize(1, 0, &n) == -1 && n == 7);
    CHECK(sh_authorize(0, 1, &n) == 0 && n == 8);
    n = UINT_MAX;
    CHECK(sh_authorize(1, 1, &n) == -1 && n == UINT_MAX);
    puts("OBS policy valid data-only unauthorized7to8 repaired");
    return 0;
}

static int maps(void)
{
    FILE *f = fopen("/proc/self/maps", "r");
    char line[1024];
    CHECK(f != NULL);
    while (fgets(line, sizeof line, f))
        CHECK(fputs(line, stdout) >= 0);
    CHECK(!ferror(f));
    CHECK(fclose(f) == 0);
    return 0;
}

static int bench(void)
{
    struct timespec a, b;
    unsigned out = 0;
    unsigned long long checksum = 0;
    for (unsigned sample = 0; sample < 31; ++sample)
    {
        CHECK(clock_gettime(CLOCK_MONOTONIC, &a) == 0);
        for (unsigned i = 0; i < 200000; ++i)
        {
            CHECK(sh_copy(payload, 8, &out) == 0);
            checksum += out;
        }
        CHECK(clock_gettime(CLOCK_MONOTONIC, &b) == 0);
        long long ns = (long long)(b.tv_sec - a.tv_sec) * 1000000000LL +
                       b.tv_nsec - a.tv_nsec;
        CHECK(ns >= 0);
        printf("SAMPLE %u %lld\n", sample, ns);
    }
    CHECK(checksum == 223200000ULL);
    printf("CHECKSUM %llu\n", checksum);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "bounds"))
        return valid();
    if (!strcmp(argv[1], "policy"))
        return policy();
    if (!strcmp(argv[1], "maps"))
        return maps();
    if (!strcmp(argv[1], "bench"))
        return bench();
    if (!strcmp(argv[1], "defect"))
    {
        volatile size_t count = 9;
        printf("unexpected %u\n", sh_flawed_copy(payload, count));
        return 3;
    }
    return 2;
}
