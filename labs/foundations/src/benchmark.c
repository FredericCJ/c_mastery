#define _POSIX_C_SOURCE 200809L
#include "foundation.h"
#include <stdio.h>
#include <time.h>
static volatile uint64_t consumed;

static uint64_t ns(struct timespec t)
{
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

int main(void)
{
    struct timespec begin, end, cpu_begin, cpu_end;
    struct f_work work = {0};
    struct f_record records[32];
    uint64_t comparisons = 0, shifts = 0;
    const unsigned repeats = 2000;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) ||
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_begin))
    {
        return 2;
    }
    for (unsigned run = 0; run < repeats; run++)
    {
        for (unsigned i = 0; i < 32; i++)
        {
            records[i].key = (int32_t)(31 - i);
            records[i].ordinal = i;
        }
        enum f_status status = f_sort(records, 32, &work);
        if (status != F_OK)
        {
            fprintf(stderr, "%s\n", f_status_name(status));
            return 1;
        }
        consumed += (uint64_t)records[31].key;
        comparisons += work.comparisons;
        shifts += work.shifts;
    }
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_end) ||
        clock_gettime(CLOCK_MONOTONIC, &end))
    {
        return 2;
    }
    printf(
        "{\"n\":32,\"repeats\":%u,\"elapsed_ns\":%llu,\"process_cpu_ns\":%llu,"
        "\"comparisons\":%llu,\"shifts\":%llu,\"consumed\":%llu,\"boundary\":"
        "\"whole batch including setup and counters\"}\n",
        repeats, (unsigned long long)(ns(end) - ns(begin)),
        (unsigned long long)(ns(cpu_end) - ns(cpu_begin)),
        (unsigned long long)comparisons, (unsigned long long)shifts,
        (unsigned long long)consumed);
    return 0;
}
