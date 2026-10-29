#include "oslab.h"

int os_simulate(const os_job *jobs, size_t count, os_policy policy,
                unsigned quantum, os_schedule *out)
{
    (void)jobs;
    (void)count;
    (void)policy;
    (void)quantum;
    (void)out;
    return -1; /* Implement the model described in SPEC.md. */
}

size_t os_clock_faults(const uint8_t *references, size_t count, size_t frames)
{
    (void)references;
    (void)count;
    (void)frames;
    return SIZE_MAX;
}
