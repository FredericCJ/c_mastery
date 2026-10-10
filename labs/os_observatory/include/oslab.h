#ifndef CMASTERY_OSLAB_H
#define CMASTERY_OSLAB_H
#include <stddef.h>
#include <stdint.h>
#define OS_MAX_JOBS 16u
#define OS_MAX_TICKS 256u
typedef enum { OS_FCFS, OS_SJF, OS_SRTF, OS_RR, OS_MLFQ, OS_STRIDE } os_policy;
typedef struct { unsigned arrival, service, weight; } os_job;
typedef struct {
    int timeline[OS_MAX_TICKS];
    unsigned ticks, dispatches, completed;
    unsigned first[OS_MAX_JOBS], finish[OS_MAX_JOBS], waiting[OS_MAX_JOBS];
} os_schedule;
/* All times are integer model ticks, not physical cycles. Returns 0 or -1 for
   invalid/unrepresentable input. Jobs have positive service and weight 1..16. */
int os_simulate(const os_job *jobs, size_t count, os_policy policy,
                unsigned quantum, os_schedule *out);
/* CLOCK second-chance replacement. references are page IDs, not addresses.
   frames 1..16; returns SIZE_MAX for invalid input. */
size_t os_clock_faults(const uint8_t *references, size_t count, size_t frames);
/* Linux hosted experiments return 0 only after validating the stated boundary. */
int os_process_probe(void);
int os_thread_probe(void);
int os_descriptor_probe(void);
int os_signal_probe(void);
int os_memory_probe(void);
int os_io_probe(void);
#endif
