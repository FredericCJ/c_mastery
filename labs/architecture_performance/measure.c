#define _GNU_SOURCE
#include "performance.h"
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef int (*pe_kernel_fn)(const uint32_t *, uint32_t, uint32_t, uint32_t *);
static volatile uint64_t observable;
static volatile unsigned char displaced[8u * 1024u * 1024u];

static uint64_t now(clockid_t which)
{
    struct timespec t;
    if (clock_gettime(which, &t) || t.tv_sec < 0 || t.tv_nsec < 0 ||
        t.tv_nsec >= 1000000000L ||
        (uint64_t)t.tv_sec > UINT64_MAX / 1000000000u)
        exit(43);
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

static uint64_t batch(pe_kernel_fn f, const uint32_t *a, uint32_t n,
                      const uint32_t *keys, unsigned repetitions)
{
#ifdef PE_MUTANT_NO_WORK
    (void)f;
    (void)a;
    (void)n;
    (void)keys;
    (void)repetitions;
    return 0;
#else
    uint64_t sum = 0;
    for (unsigned r = 0; r < repetitions; ++r)
        for (unsigned i = 0; i < 64; ++i)
        {
            uint32_t result = UINT32_MAX;
            int rc = f(a, n, keys[i], &result);
            sum += (uint64_t)result + (uint64_t)(unsigned)rc * 1048576u;
        }
    return sum;
#endif
}

static void prepare(unsigned policy, pe_kernel_fn f, const uint32_t *a,
                    uint32_t n, const uint32_t *keys)
{
    if (policy)
    {
        for (size_t i = 0; i < sizeof displaced; i += 64)
            displaced[i] = (unsigned char)(displaced[i] + 1);
    }
    else
        observable ^= batch(f, a, n, keys, 1);
}

int main(void)
{
    struct timespec resolution;
    if (clock_getres(CLOCK_MONOTONIC_RAW, &resolution))
        return 43;
    uint64_t res = (uint64_t)resolution.tv_sec * 1000000000u +
                   (uint64_t)resolution.tv_nsec,
             overhead[101];
    for (unsigned i = 0; i < 101; ++i)
    {
        uint64_t start = now(CLOCK_MONOTONIC_RAW),
                 end = now(CLOCK_MONOTONIC_RAW);
        if (end < start)
            return 43;
        overhead[i] = end - start;
        printf("OVERHEAD %u %" PRIu64 "\n", i, overhead[i]);
    }
    struct pe_stats overhead_stats;
    if (pe_stats(overhead, 101, &overhead_stats))
        return 41;
    uint64_t target = 50000;
    if (res <= UINT64_MAX / 1000 && target < res * 1000)
        target = res * 1000;
    if (overhead_stats.maximum <= UINT64_MAX / 100 &&
        target < overhead_stats.maximum * 100)
        target = overhead_stats.maximum * 100;
    printf("CLOCK %" PRIu64 " %" PRIu64 "\n", res, target);
    const uint32_t sizes[] = {16, 1024};
    pe_kernel_fn kernels[] = {pe_c, pe_asm};
    uint32_t a[1024], keys[64];
    for (unsigned family = 0; family < 2; ++family)
    {
        uint32_t n = sizes[family];
        for (uint32_t i = 0; i < n; ++i)
            a[i] = (i / 2) * 8388608u;
        uint64_t expected = 0, comparisons = 0;
        for (unsigned i = 0; i < 64; ++i)
        {
            keys[i] = i == 63 ? UINT32_MAX
                              : (uint32_t)((uint64_t)i * UINT32_MAX / 63);
            uint32_t index = 0;
            while (index < n && a[index] < keys[i])
                ++index;
            expected += index;
            algo_work w = {0};
            uint32_t actual = 0;
            if (algo_lower(a, n, keys[i], &actual, &w) || actual != index)
                return 41;
            comparisons += w.comparisons;
            for (unsigned k = 0; k < 2; ++k)
            {
                actual = UINT32_MAX;
                if (kernels[k](a, n, keys[i], &actual) || actual != index)
                    return 41;
            }
        }
        printf("WORK %u %" PRIu64 " %u %" PRIu64 "\n", n, comparisons,
               64 * (n - 1), expected);
        for (unsigned policy = 0; policy < 2; ++policy)
        {
            unsigned repetitions = 1;
            for (;;)
            {
                uint64_t shortest = UINT64_MAX;
                for (unsigned k = 0; k < 2; ++k)
                {
                    prepare(policy, kernels[k], a, n, keys);
                    uint64_t start = now(CLOCK_MONOTONIC_RAW),
                             sum = batch(kernels[k], a, n, keys, repetitions),
                             end = now(CLOCK_MONOTONIC_RAW);
                    if (end < start || sum != expected * repetitions)
                    {
                        fputs(
                            "PE_MEASURE_ASSERT: calibration result/checksum\n",
                            stderr);
                        return 41;
                    }
                    if (end - start < shortest)
                        shortest = end - start;
                    observable ^= sum;
                }
                if (shortest >= target || repetitions == 512)
                    break;
                repetitions *= 2;
            }
            for (unsigned block = 0; block < 31; ++block)
                for (unsigned order = 0; order < 2; ++order)
                {
                    unsigned k = order ^ (block & 1u);
                    prepare(policy, kernels[k], a, n, keys);
                    int before = sched_getcpu();
                    uint64_t cpu0 = now(CLOCK_THREAD_CPUTIME_ID),
                             start = now(CLOCK_MONOTONIC_RAW),
                             sum = batch(kernels[k], a, n, keys, repetitions),
                             end = now(CLOCK_MONOTONIC_RAW),
                             cpu1 = now(CLOCK_THREAD_CPUTIME_ID);
                    int after = sched_getcpu();
                    if (end < start || cpu1 < cpu0 ||
                        sum != expected * repetitions)
                        return 41;
                    observable ^= sum;
                    printf("SAMPLE %u %u %u %u %u %u %" PRIu64 " %" PRIu64
                           " %d %d %" PRIu64 "\n",
                           n, policy, block, order, k, repetitions, end - start,
                           cpu1 - cpu0, before, after, sum);
                }
        }
    }
    puts("PE_MEASURE_PASS raw=248 no_speed_threshold");
    return 0;
}
