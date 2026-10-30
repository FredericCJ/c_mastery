/*
 * Timing driver: one CSV line per sample comparing os_transform (restrict)
 * with os_ordinary (no restrict) on identical, disjoint buffers.
 *
 * Line format: variant,bytes,pattern,offset,sample,repetitions,elapsed_ns
 *   variant 1 = os_transform, 0 = os_ordinary (alternated so that neither
 *   always runs first); pattern 1 = pseudo-random input, 0 = all zero;
 *   offset = bytes added to the 64-byte-aligned buffers (0 or 1).
 * Exit status: 0 success; 2 clock failure; 41 output differs from the XOR
 * reference (marker OS_BENCH_ASSERT:). Only correctness is asserted here;
 * the timings are raw observations, not a speed claim.
 */
#define _POSIX_C_SOURCE 200809L
#include "alias.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

enum
{
    BUFFER_BYTES = 8192,
    BUFFER_ALIGNMENT_BYTES = 64,
    SAMPLES = 15,
    NS_PER_SECOND = 1000000000,
    XOR_MASK = 0x55
};

/* Linear congruential generator (Numerical Recipes constants), fixed seed. */
#define LCG_MULTIPLIER 1664525u
#define LCG_INCREMENT 1013904223u
#define LCG_SEED 17u

struct bench_size
{
    size_t bytes;
    unsigned repetitions; /* inner-loop calls per timed sample */
};

/* Short inputs need many more repetitions to rise above clock resolution. */
static const struct bench_size sizes[] = {{64, 10000}, {4096, 1000}};

static _Alignas(BUFFER_ALIGNMENT_BYTES) unsigned char source[BUFFER_BYTES];
static _Alignas(BUFFER_ALIGNMENT_BYTES) unsigned char destination[BUFFER_BYTES];

/* Reads CLOCK_MONOTONIC as nanoseconds. Returns 0 on success, -1 on failure
 * (*ns is then unchanged). */
static int now_ns(uint64_t *ns)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0)
    {
        return -1;
    }
    *ns = (uint64_t)t.tv_sec * NS_PER_SECOND + (uint64_t)t.tv_nsec;
    return 0;
}

/* Fills source[offset, offset + bytes) and clears the same range of the
 * destination. */
static void fill_buffers(size_t offset, size_t bytes, int random_pattern)
{
    uint32_t state = LCG_SEED;
    for (size_t i = 0; i < bytes; ++i)
    {
        state = state * LCG_MULTIPLIER + LCG_INCREMENT;
        /* The high byte of an LCG state has the longest period. */
        source[offset + i] = random_pattern ? (unsigned char)(state >> 24) : 0;
        destination[offset + i] = 0;
    }
}

static void apply_variant(int use_restrict, size_t offset, size_t bytes,
                          unsigned repetitions)
{
    for (unsigned k = 0; k < repetitions; ++k)
    {
        if (use_restrict)
        {
            os_transform(destination + offset, source + offset, bytes,
                         XOR_MASK);
        }
        else
        {
            os_ordinary(destination + offset, source + offset, bytes, XOR_MASK);
        }
    }
}

static int output_matches_reference(size_t offset, size_t bytes)
{
    for (size_t i = 0; i < bytes; ++i)
    {
        if (destination[offset + i] !=
            (unsigned char)(source[offset + i] ^ XOR_MASK))
        {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    for (size_t size_index = 0; size_index < 2; ++size_index)
    {
        const size_t bytes = sizes[size_index].bytes;
        const unsigned repetitions = sizes[size_index].repetitions;
        for (unsigned pattern = 0; pattern < 2; ++pattern)
        {
            for (unsigned offset = 0; offset < 2; ++offset)
            {
                for (unsigned sample = 0; sample < SAMPLES; ++sample)
                {
                    for (unsigned order = 0; order < 2; ++order)
                    {
                        const unsigned variant = (sample + order) & 1u;
                        uint64_t before = 0;
                        uint64_t after = 0;
                        fill_buffers(offset, bytes, (int)pattern);
                        if (now_ns(&before) != 0)
                        {
                            return 2;
                        }
                        apply_variant((int)variant, offset, bytes, repetitions);
                        if (now_ns(&after) != 0 || after < before)
                        {
                            return 2;
                        }
                        if (!output_matches_reference(offset, bytes))
                        {
                            puts("OS_BENCH_ASSERT:equivalent admitted output");
                            return 41;
                        }
                        printf("%u,%zu,%u,%u,%u,%u,%llu\n", variant, bytes,
                               pattern, offset, sample, repetitions,
                               (unsigned long long)(after - before));
                    }
                }
            }
        }
    }
    return 0;
}
