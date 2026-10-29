#define _POSIX_C_SOURCE 200809L
#include "workload.h"
#include <stdio.h>
#include <time.h>
static struct sf_work work;

int main(void)
{
    for (unsigned burst = 1; burst <= 4; burst += 3)
    {
        struct timespec a, b;
        clock_gettime(CLOCK_MONOTONIC, &a);
        int r = sfw_run(&work, burst);
        clock_gettime(CLOCK_MONOTONIC, &b);
        if (r)
            return r;
        long long elapsed =
            (b.tv_sec - a.tv_sec) * 1000000000LL + b.tv_nsec - a.tv_nsec;
        printf("SFW %u %u %u %u %u %u %u %lld host-monotonic-ns\n", burst,
               work.offered, work.admitted, work.rejected, work.completed,
               work.uncertain, work.checksum, elapsed);
    }
    return 0;
}
