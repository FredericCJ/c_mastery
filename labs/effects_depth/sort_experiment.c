/*
 * Timing experiment: the bounded insertion sort against the C library qsort
 * on 16 integers (25 samples of 2000 sorts), then qsort on 16 large records.
 * Output is one JSON object; timing is in nanoseconds from CLOCK_MONOTONIC.
 * Record it with the host, compiler and optimization level it was taken on.
 *
 * Exit status: abort() when the clock fails, a sort is rejected or the
 * records do not end up in ascending key order.
 */
#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SAMPLES 25u
#define SORTS_PER_SAMPLE 2000u
#define ITEMS 16u
#define NANOSECONDS_PER_SECOND UINT64_C(1000000000)

static unsigned comparisons;

static int compare_int(const void *left, const void *right)
{
    ++comparisons;
    int x = *(const int *)left;
    int y = *(const int *)right;
    return (x > y) - (x < y);
}

/* Compares keys but also reads both payloads, as a large record would. */
static int compare_record(const void *left, const void *right)
{
    ++comparisons;
    const struct ef_record *x = left;
    const struct ef_record *y = right;
    volatile unsigned checksum = 0;
    for (unsigned i = 0; i < EF_RECORD_PAYLOAD_BYTES; ++i)
    {
        checksum += x->payload[i] + y->payload[i];
    }
    (void)checksum;
    return (x->key > y->key) - (x->key < y->key);
}

static uint64_t now_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * NANOSECONDS_PER_SECOND + (uint64_t)t.tv_nsec;
}

/* Deterministic input with repeated values (7 distinct), varying per repeat. */
static void fill_input(int values[ITEMS], unsigned repeat)
{
    for (unsigned i = 0; i < ITEMS; ++i)
    {
        values[i] = (int)((i * 11 + repeat) % 7);
    }
}

/* Elapsed nanoseconds for SORTS_PER_SAMPLE bounded sorts. */
static uint64_t time_bounded_sorts(unsigned *checksum)
{
    uint64_t start = now_ns();
    for (unsigned repeat = 0; repeat < SORTS_PER_SAMPLE; ++repeat)
    {
        int values[ITEMS];
        fill_input(values, repeat);
        if (ef_sort(values, ITEMS) != EF_OK)
        {
            abort();
        }
        *checksum += (unsigned)values[0];
    }
    return now_ns() - start;
}

/* Elapsed nanoseconds for SORTS_PER_SAMPLE qsort calls. */
static uint64_t time_library_sorts(unsigned *checksum)
{
    uint64_t start = now_ns();
    for (unsigned repeat = 0; repeat < SORTS_PER_SAMPLE; ++repeat)
    {
        int values[ITEMS];
        fill_input(values, repeat);
        qsort(values, ITEMS, sizeof *values, compare_int);
        *checksum += (unsigned)values[0];
    }
    return now_ns() - start;
}

/* Sorts 16 descending-key records with qsort; aborts if the keys are wrong. */
static uint64_t time_large_records(void)
{
    struct ef_record records[ITEMS];
    for (unsigned i = 0; i < ITEMS; ++i)
    {
        records[i].key = (int)(ITEMS - 1 - i);
        records[i].identity = i;
        memset(records[i].payload, (int)i, EF_RECORD_PAYLOAD_BYTES);
    }
    comparisons = 0;
    uint64_t start = now_ns();
    qsort(records, ITEMS, sizeof *records, compare_record);
    uint64_t elapsed = now_ns() - start;
    for (unsigned i = 0; i < ITEMS; ++i)
    {
        if (records[i].key != (int)i)
        {
            abort();
        }
    }
    return elapsed;
}

int main(void)
{
    unsigned checksum = 0;
    printf("{\"samples\":[");
    for (unsigned sample = 0; sample < SAMPLES; ++sample)
    {
        uint64_t bounded_ns = time_bounded_sorts(&checksum);
        comparisons = 0;
        uint64_t library_ns = time_library_sorts(&checksum);
        printf(
            "%s{\"bounded_ns\":%llu,\"qsort_ns\":%llu,\"qsort_comparisons\":%u}",
            sample ? "," : "", (unsigned long long)bounded_ns,
            (unsigned long long)library_ns, comparisons);
    }
    uint64_t large_ns = time_large_records();
    printf(
        "],\"large_record_ns\":%llu,\"large_record_comparisons\":%u,\"checksum\":%u,\"contract\":\"ascending keys and permutation; qsort stability not assumed\"}\n",
        (unsigned long long)large_ns, comparisons, checksum);
    return 0;
}
