#define _POSIX_C_SOURCE 200809L
#include "numeric_extension.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>
static double a[4096], b[4096], out[4096];
static unsigned char eviction[8 * 1024 * 1024];
static volatile uint64_t sink;

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
    size_t sizes[] = {7, 16, 31, 64};
    for (size_t i = 0; i < 4096; i++)
    {
        a[i] = (double)((int)(i % 15) - 7);
        b[i] = (double)((int)((i * 3) % 15) - 7);
    }
    puts("size,mode,eviction_sweep,sample,nanoseconds,repetitions,checksum");
    for (unsigned z = 0; z < 4; z++)
    {
        for (unsigned mode = 0; mode < 3; mode++)
        {
            for (unsigned cold = 0; cold < 2; cold++)
            {
                for (unsigned sample = 0; sample < 9; sample++)
                {
                    size_t n = sizes[z];
                    if (cold)
                    {
                        for (size_t i = 0; i < sizeof eviction; i += 64)
                        {
                            eviction[i]++;
                            sink += eviction[i];
                        }
                    }
                    unsigned reps = n < 32 ? 32 : 4;
                    uint64_t begin = now();
                    for (unsigned r = 0; r < reps; r++)
                    {
                        if (!nd_matrix(a, b, n, n, n, out, 4096, mode))
                        {
                            return 2;
                        }
                    }
                    uint64_t end = now();
                    double sum = 0;
                    for (size_t i = 0; i < n * n; i++)
                    {
                        sum += out[i];
                    }
                    printf("%zu,%u,%u,%u,%llu,%u,%.0f\n", n, mode, cold, sample,
                           (unsigned long long)(end - begin), reps, sum);
                }
            }
        }
    }
    return 0;
}
