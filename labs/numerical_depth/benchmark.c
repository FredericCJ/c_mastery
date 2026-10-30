/*
 * Host timing of an array-of-structures (AoS) against a structure-of-arrays
 * (SoA) layout for the same data, for an x-only kernel and an all-fields
 * kernel. Prints one JSON object per measurement; the result is checked
 * against the AoS value so both layouts compute the identical sum.
 *
 * Timing is wall-clock nanoseconds from CLOCK_MONOTONIC over CALLS_PER_SAMPLE
 * calls. It is an observation of this host and build, not a cache-miss count
 * and not a universal speed ranking. Exit status 1 when a layout's result
 * differs from the AoS reference.
 */
#define _POSIX_C_SOURCE 200809L
#include "numeric.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define RECORDS 4096u
#define CALLS_PER_SAMPLE 256u
#define REPEATS 9u
#define SMALLEST_RECORD_COUNT 16u
#define RECORD_COUNT_STEP 16u
#define NANOSECONDS_PER_SECOND UINT64_C(1000000000)

static struct nd_record aos[RECORDS];
static double xs[RECORDS];
static double ys[RECORDS];
static char labels[RECORDS][ND_LABEL_BYTES];

/* Keeps the compiler from discarding the kernel result. */
static volatile double sink;

/* Monotonic time in nanoseconds; returns 0 if the clock cannot be read (the
 * benchmark does not detect that case). */
static uint64_t now_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        return 0;
    }
    return (uint64_t)t.tv_sec * NANOSECONDS_PER_SECOND + (uint64_t)t.tv_nsec;
}

/*
 * Sums the first n records from the AoS or the SoA copy. With all_fields it
 * also adds y and every label byte. Not inlined, so each layout is measured
 * as a call and its generated code can be inspected on its own. The additions
 * run in index order; the order is part of the result.
 */
__attribute__((noinline)) static double run_kernel(size_t n, int soa,
                                                   int all_fields)
{
    double sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        sum += soa ? xs[i] : aos[i].x;
        if (all_fields)
        {
            sum += soa ? ys[i] : aos[i].y;
            for (size_t k = 0; k < ND_LABEL_BYTES; k++)
            {
                sum += soa ? labels[i][k] : aos[i].label[k];
            }
        }
    }
    return sum;
}

/* Fills both layouts with the same deterministic values. */
static void fill_data(void)
{
    for (size_t i = 0; i < RECORDS; i++)
    {
        aos[i].x = xs[i] = (double)(i % 16);
        aos[i].y = ys[i] = (double)(i % 8);
        for (size_t k = 0; k < ND_LABEL_BYTES; k++)
        {
            aos[i].label[k] = labels[i][k] = (char)(k % 4);
        }
    }
}

/*
 * Times CALLS_PER_SAMPLE calls of one kernel and prints the measurement.
 * Returns 1 if the kernel's result differs from the AoS reference, else 0.
 */
static int measure(size_t n, int all_fields, unsigned repeat, int soa)
{
    double reference = run_kernel(n, 0, all_fields);
    double value = 0;
    uint64_t start = now_ns();
    for (unsigned i = 0; i < CALLS_PER_SAMPLE; i++)
    {
        /* Compiler barrier: forces the arrays to be re-read on each call. */
        __asm__ __volatile__("" : : "g"(xs), "g"(aos) : "memory");
        value = run_kernel(n, soa, all_fields);
        sink = value;
    }
    uint64_t elapsed_ns = now_ns() - start;
    if (value != reference)
    {
        return 1;
    }
    printf("{\"n\":%zu,\"all_fields\":%d,\"soa\":%d,\"repeat\":%u,\"ns\":"
           "%llu,\"result\":\"%a\"}\n",
           n, all_fields, soa, repeat, (unsigned long long)elapsed_ns, value);
    return 0;
}

int main(void)
{
    fill_data();
    for (size_t n = SMALLEST_RECORD_COUNT; n <= RECORDS; n *= RECORD_COUNT_STEP)
    {
        for (int all_fields = 0; all_fields < 2; all_fields++)
        {
            for (unsigned repeat = 0; repeat < REPEATS; repeat++)
            {
                for (int soa = 0; soa < 2; soa++)
                {
                    if (measure(n, all_fields, repeat, soa))
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}
