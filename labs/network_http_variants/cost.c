#define _POSIX_C_SOURCE 200809L
#include "variant.h"
#include "http_depth.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        return 0;
    }
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

int main(void)
{
    char a[30], b[30];
    size_t n, m;
    unsigned total = 0;
    for (unsigned sample = 0; sample < 7; sample++)
    {
        for (unsigned i = 0; i < 512; i++)
        {
            uint32_t day =
                (uint32_t)(((uint64_t)i * 5729 + sample * 311) % 2932897);
            if (nv_date(1, day, 0, a, 30, &n) ||
                hd_date(1, day, 0, b, 30, &m) || n != m || memcmp(a, b, 30))
            {
                return 1;
            }
        }
        uint64_t start = now();
        for (unsigned i = 0; i < 512; i++)
        {
            uint32_t day =
                (uint32_t)(((uint64_t)i * 5729 + sample * 311) % 2932897);
            if (nv_date(1, day, 0, a, 30, &n))
            {
                return 1;
            }
            total += (unsigned char)a[0];
        }
        uint64_t middle = now();
        for (unsigned i = 0; i < 512; i++)
        {
            uint32_t day =
                (uint32_t)(((uint64_t)i * 5729 + sample * 311) % 2932897);
            if (hd_date(1, day, 0, a, 30, &n))
            {
                return 1;
            }
            total += (unsigned char)a[0];
        }
        uint64_t end = now();
        if (!start || middle < start || end < middle)
        {
            return 1;
        }
        printf(
            "NV_COST:%u:cycle_ns=%llu:iterative_ns=%llu:512-equivalent-dates\n",
            sample, (unsigned long long)(middle - start),
            (unsigned long long)(end - middle));
    }
    printf("NV_OK:cost:%u\n", total);
    return 0;
}
