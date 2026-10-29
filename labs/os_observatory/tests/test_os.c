#include "oslab.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static void trace(os_policy policy, const os_job *jobs, size_t n,
                  unsigned quantum, const int *expected, size_t ticks)
{
    os_schedule s;
    assert(os_simulate(jobs, n, policy, quantum, &s) == 0);
    assert(s.ticks == ticks && s.completed == n);
    for (size_t i = 0; i < ticks; ++i)
    {
        assert(s.timeline[i] == expected[i]);
    }
    for (size_t i = 0; i < n; ++i)
    {
        unsigned service = 0;
        for (size_t t = 0; t < ticks; ++t)
        {
            service += s.timeline[t] == (int)i;
        }
        assert(service == jobs[i].service && s.first[i] >= jobs[i].arrival);
        assert(s.waiting[i] + jobs[i].service == s.finish[i] - jobs[i].arrival);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const char *which = argv[1];
    if (!strcmp(which, "schedulers"))
    {
        os_job jobs[] = {{0, 4, 1}, {1, 1, 1}, {2, 2, 1}};
        int fcfs[] = {0, 0, 0, 0, 1, 2, 2}, srtf[] = {0, 1, 2, 2, 0, 0, 0},
            rr[] = {0, 0, 1, 2, 2, 0, 0};
        trace(OS_FCFS, jobs, 3, 2, fcfs, 7);
        trace(OS_SJF, jobs, 3, 2, fcfs, 7);
        trace(OS_SRTF, jobs, 3, 2, srtf, 7);
        trace(OS_RR, jobs, 3, 2, rr, 7);
        os_job simultaneous[] = {{0, 3, 1}, {0, 1, 1}, {0, 2, 1}};
        int sjf[] = {1, 2, 2, 0, 0, 0};
        trace(OS_SJF, simultaneous, 3, 1, sjf, 6);
        os_schedule s;
        assert(os_simulate(NULL, 0, OS_RR, 1, &s) == 0 && s.ticks == 0);
        assert(os_simulate(jobs, 3, OS_RR, 0, &s) == -1);
        os_job invalid[] = {{255, 2, 1}};
        assert(os_simulate(invalid, 1, OS_FCFS, 1, &s) == -1);
        os_job idle[] = {{2, 1, 1}};
        int it[] = {-1, -1, 0};
        trace(OS_FCFS, idle, 1, 1, it, 3);
    }
    else if (!strcmp(which, "fairness"))
    {
        os_job jobs[] = {{0, 4, 1}, {0, 4, 1}};
        int mlfq[] = {0, 1, 0, 0, 1, 1, 0, 1};
        trace(OS_MLFQ, jobs, 2, 1, mlfq, 8);
        os_job weighted[] = {{0, 8, 1}, {0, 16, 2}};
        os_schedule s;
        assert(!os_simulate(weighted, 2, OS_STRIDE, 1, &s));
        unsigned a = 0, b = 0;
        for (unsigned t = 0; t < 12; ++t)
        {
            a += s.timeline[t] == 0;
            b += s.timeline[t] == 1;
        }
        assert(a == 4 && b == 8 && s.completed == 2);
    }
    else if (!strcmp(which, "clock"))
    {
        uint8_t refs[] = {1, 2, 3, 1, 4, 1, 2, 5};
        assert(os_clock_faults(refs, 8, 3) == 7);
        assert(os_clock_faults(refs, 8, 16) == 5);
        /* After 4 clears old reference bits, the hit on 2 gives it a second
           chance. FIFO wrongly evicts 2 and faults again on the final access. */
        uint8_t discriminate[] = {1, 2, 3, 4, 2, 5, 2};
        assert(os_clock_faults(discriminate, 7, 3) == 5);
        uint8_t repeat[] = {0, 0, 0};
        assert(os_clock_faults(repeat, 3, 1) == 1);
        assert(os_clock_faults(NULL, 0, 1) == 0 &&
               os_clock_faults(NULL, 1, 1) == SIZE_MAX);
        assert(os_clock_faults(refs, 8, 0) == SIZE_MAX);
    }
    else if (!strcmp(which, "process"))
    {
        assert(!os_process_probe());
    }
    else if (!strcmp(which, "thread"))
    {
        assert(!os_thread_probe());
    }
    else if (!strcmp(which, "descriptor"))
    {
        assert(!os_descriptor_probe());
    }
    else if (!strcmp(which, "signal"))
    {
        assert(!os_signal_probe());
    }
    else if (!strcmp(which, "memory"))
    {
        assert(!os_memory_probe());
    }
    else if (!strcmp(which, "io"))
    {
        assert(!os_io_probe());
    }
    else
    {
        return 2;
    }
    puts("OS boundary accepted for this finite experiment");
    return 0;
}
