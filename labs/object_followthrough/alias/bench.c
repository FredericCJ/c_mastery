#define _POSIX_C_SOURCE 200809L
#include "alias.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
static _Alignas(64) unsigned char source[8192], destination[8192];

static uint64_t stamp(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
        return 0;
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

int main(void)
{
    for (unsigned size_case = 0; size_case < 2; ++size_case)
        for (unsigned pattern = 0; pattern < 2; ++pattern)
            for (unsigned offset = 0; offset < 2; ++offset)
                for (unsigned sample = 0; sample < 15; ++sample)
                    for (unsigned order = 0; order < 2; ++order)
                    {
                        size_t n = size_case ? 4096 : 64;
                        unsigned repetitions = size_case ? 1000 : 10000;
                        unsigned variant = (sample + order) & 1u;
                        uint32_t rng = 17;
                        for (size_t i = 0; i < n; ++i)
                        {
                            rng = rng * 1664525u + 1013904223u;
                            source[offset + i] =
                                pattern ? (unsigned char)(rng >> 24) : 0;
                            destination[offset + i] = 0;
                        }
                        uint64_t before = stamp();
                        if (!before)
                            return 2;
                        for (unsigned k = 0; k < repetitions; ++k)
                        {
                            if (variant)
                                os_transform(destination + offset,
                                             source + offset, n, 0x55);
                            else
                                os_ordinary(destination + offset,
                                            source + offset, n, 0x55);
                        }
                        uint64_t after = stamp();
                        if (after < before)
                            return 2;
                        for (size_t i = 0; i < n; ++i)
                            if (destination[offset + i] !=
                                (unsigned char)(source[offset + i] ^ 0x55))
                            {
                                puts(
                                    "OS_BENCH_ASSERT:equivalent admitted output");
                                return 41;
                            }
                        printf("%u,%zu,%u,%u,%u,%u,%llu\n", variant, n, pattern,
                               offset, sample, repetitions,
                               (unsigned long long)(after - before));
                    }
    return 0;
}
