#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    uint32_t *p = malloc(4096 * sizeof *p);
    if (!p)
        return 2;
    uint64_t sum = 0;
    struct timespec start, end, res;
    if (clock_getres(CLOCK_MONOTONIC, &res) ||
        clock_gettime(CLOCK_MONOTONIC, &start))
        return 2;
    for (unsigned r = 0; r < 512; r++)
    {
        for (unsigned i = 0; i < 4096; i++)
            p[i] = (i * 17u + r) % 65521u;
        for (unsigned i = 0; i < 4096; i++)
            sum += p[i];
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end))
        return 2;
    free(p);
    printf("WORKLOAD:%llu:ns=%lld:resolution_ns=%ld\n", (unsigned long long)sum,
           (long long)(end.tv_sec - start.tv_sec) * 1000000000LL + end.tv_nsec -
               start.tv_nsec,
           res.tv_nsec);
    return 0;
}
