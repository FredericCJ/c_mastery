#define _POSIX_C_SOURCE 200809L
#include "allocation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

int main(void)
{
    for (unsigned policy = 0; policy < 2; ++policy)
    {
        for (unsigned instrumentation = 0; instrumentation < 2;
             ++instrumentation)
        {
            for (unsigned workload = 0; workload < 3; ++workload)
            {
                for (unsigned sample = 0; sample < 9; ++sample)
                {
                    struct al_heap h;
                    struct al_handle handles[8];
                    uint64_t start = now();
                    size_t peak = 0, charged = 0, big = 0, total = 0;
                    unsigned failures = 0;
                    for (unsigned repeat = 0; repeat < 128; ++repeat)
                    {
                        if (al_heap_init(&h, 256, policy, 0, 1))
                        {
                            abort();
                        }
                        for (unsigned i = 0; i < 8; ++i)
                        {
                            unsigned index = workload == 2 ? 7 - i : i;
                            size_t n = index % 2 ? 4 : 20;
                            if (al_heap_take(&h, n, 8, &handles[i]))
                            {
                                abort();
                            }
                            if (instrumentation && al_heap_check(&h))
                            {
                                abort();
                            }
                        }
                        for (unsigned i = 0; i < 8; i += 2)
                        {
                            unsigned index = workload == 1 ? i / 2 : i;
                            if (al_heap_release(&h, handles[index]))
                            {
                                abort();
                            }
                            if (instrumentation && al_heap_check(&h))
                            {
                                abort();
                            }
                        }
                        struct al_handle request;
                        int s = al_heap_take(&h, 60, 8, &request);
                        if (s == AL_FULL)
                        {
                            ++failures;
                        }
                        else if (s != AL_OK)
                        {
                            abort();
                        }
                        if (al_heap_stats(&h, &total, &big))
                        {
                            abort();
                        }
                        peak = h.peak_payload;
                        charged = h.peak_charged;
                    }
                    uint64_t elapsed = now() - start;
                    printf(
                        "{\"policy\":%u,\"extra_invariant_instrumentation\":%u,"
                        "\"workload\":%u,\"sample\":%u,\"repetitions\":128,\"ns\":%llu,"
                        "\"peak_payload\":%zu,\"peak_charged\":%zu,\"current_payload\":%"
                        "zu,\"current_charged\":%zu,\"retained_region\":256,\"total_"
                        "free\":%zu,\"largest_free\":%zu,\"failures\":%u}\n",
                        policy, instrumentation, workload, sample,
                        (unsigned long long)elapsed, peak, charged, h.payload,
                        h.charged, total, big, failures);
                }
            }
        }
    }
    return 0;
}
