#define _POSIX_C_SOURCE 200809L
#include "algo.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
uint64_t pf_helper(uint64_t);

static uint64_t ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        exit(1);
    }
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned)t.tv_nsec;
}

__attribute__((noinline)) uint64_t pf_child(unsigned n)
{
    volatile uint64_t x = 1;
    for (unsigned i = 0; i < n; ++i)
    {
        x = x * 33 + i;
    }
    return x;
}

__attribute__((noinline)) uint64_t pf_expensive(void)
{
    return pf_child(2000000);
}

__attribute__((noinline)) uint64_t pf_cheap(void)
{
    return pf_child(1000);
}
#ifdef PF_HOOKS
void pf_record_begin(void);
void pf_record_end(void);
#endif
int main(void)
{
    uint32_t a[32];
    for (unsigned i = 0; i < 32; ++i)
    {
        a[i] = i * 2;
    }
    uint64_t sum = 0;
#ifdef PF_HOOKS
    pf_record_begin();
#endif
    uint64_t start = ns();
    for (unsigned i = 0; i < 1000; ++i)
    {
        uint32_t r;
        if (algo_lower(a, 32, i % 65, &r, 0))
        {
            return 1;
        }
        sum += r;
    }
    sum += pf_helper(7);
    uint64_t end = ns();
    printf("PROFILE_RUN:%llu:%llu\n", (unsigned long long)(end - start),
           (unsigned long long)sum);
    uint64_t x = ns();
    for (unsigned i = 0; i < 2; ++i)
    {
        sum ^= pf_expensive();
    }
    uint64_t y = ns();
    for (unsigned i = 0; i < 8; ++i)
    {
        sum ^= pf_cheap();
    }
    uint64_t z = ns();
    printf("CALLERS:2:%llu:8:%llu:%llu\n", (unsigned long long)(y - x),
           (unsigned long long)(z - y), (unsigned long long)sum);
#ifdef PF_HOOKS
    pf_record_end();
#endif
    return 0;
}
