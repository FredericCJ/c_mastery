#define _GNU_SOURCE
#include "bytes.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sched.h>

void ab_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AB_ASSERT:%u:%s\n", line, s);
}

void ab_emit(const char *s, const uint64_t *v, unsigned n)
{
    printf("DATA %s", s);
    for (unsigned i = 0; i < n; ++i)
        printf(" %" PRIu64, v[i]);
    putchar('\n');
}

static int allocated(void)
{
    struct aw_record *heap = malloc(sizeof(*heap));
    if (!heap)
        return 43;
    /* First typed store into actual allocated storage; no declared byte object. */
    *heap = (struct aw_record){INT32_MIN, INT32_MAX};
    struct aw_record copy = {0, 0};
    unsigned char wire[8];
    int ok = ab_record_copy(&copy, heap) == AB_OK &&
             ab_record_encode(heap, 1, wire, 8) == AB_OK &&
             copy.id == INT32_MIN && copy.rank == INT32_MAX;
    free(heap);
    if (!ok)
    {
        ab_failure("actual allocated backing/copy", __LINE__);
        return 41;
    }
    puts("OBS allocated");
    return 0;
}

static int timing(void)
{
    struct timespec resolution;
    if (clock_getres(CLOCK_MONOTONIC, &resolution))
        return 43;
    printf("CLOCK %ld %ld\n", (long)resolution.tv_sec, resolution.tv_nsec);
    for (unsigned size = 0; size < 2; ++size)
    {
        uint64_t n = size ? 100000 : 1000;
        for (unsigned kind = 0; kind < 2; ++kind)
            if (ab_dependency(kind, 123, n, 19) != 36)
                return 41;
        for (unsigned pair = 0; pair < 31; ++pair)
            for (unsigned order = 0; order < 2; ++order)
            {
                unsigned kind = (pair + order) & 1;
                struct timespec a, b;
                int cpu0 = sched_getcpu();
                if (clock_gettime(CLOCK_MONOTONIC, &a))
                    return 43;
                uint64_t v = ab_dependency(kind, 123, n, 19);
                if (clock_gettime(CLOCK_MONOTONIC, &b))
                    return 43;
                int cpu1 = sched_getcpu();
                int64_t ns =
                    (int64_t)(b.tv_sec - a.tv_sec) * INT64_C(1000000000) +
                    b.tv_nsec - a.tv_nsec;
                if (v != 36 || ns < 0)
                    return 41;
                printf("TIME %u %u %u %" PRIu64 " %" PRId64 " %d %d %" PRIu64
                       "\n",
                       size, pair, kind, n, ns, cpu0, cpu1, v);
            }
    }
    puts("OBS timing");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 42;
    if (!strcmp(argv[1], "allocated"))
        return allocated();
    if (!strcmp(argv[1], "timing"))
        return timing();
    int code = ab_test(argv[1]);
    if (!code)
        printf("OBS %s\n", argv[1]);
    return code;
}
