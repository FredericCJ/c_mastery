/*
 * JSON-lines serialization for the native collector (pmu_native.c).
 * Supplied course infrastructure.
 *
 * Each function writes exactly one complete line to `stream` and never runs
 * inside a measured interval. The field names are an interface read by
 * tools/run_native_pmu.py, tools/analyze_native_pmu.py,
 * tools/check_perf_control.py and native.py, and recorded in receipts:
 * add fields, never rename or remove one.
 *
 * Raw observations and derived values are kept apart: these lines carry raw
 * counts, times and states only. Scaled counts, ratios and rankings are
 * derived later by the gates (labs/support/measurement_gates.py).
 */
#ifndef PF_PMU_REPORT_H
#define PF_PMU_REPORT_H

#include "cm_clock.h"
#include "pmu.h"
#include "pmu_sampling.h"
#include "pmu_workloads.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* The validated request every observation line repeats. */
struct pf_request_summary
{
    enum pf_family family;
    size_t n;
    unsigned variant;
    unsigned repetitions;
};

struct pf_count_report
{
    struct pf_request_summary request;
    uint64_t checksum;
    uint64_t elapsed_ns;
    int cpu_before; /* sched_getcpu at the interval's ends; -1 if unknown */
    int cpu_after;
    enum pf_pmu_state state; /* PF_PMU_EMPTY when no group was opened */
    bool strict_group; /* counts come from this process's strict group;
                          false for `work`, where perf stat counts */
    const struct pf_pmu *group; /* borrowed for the call */
    const struct cm_clock_calibration *clock; /* borrowed for the call */
};

/* {"kind":"count",...}: one counting (or perf-control "work") batch. */
void pf_report_count(FILE *stream, const struct pf_count_report *report);

/* The smallest sample period that stays under perf_event_max_sample_rate
 * at the CPU's maximum frequency: ceil(cpu_max_khz * 1000 / rate). A shorter
 * period is bound to be throttled whenever the CPU runs near that frequency,
 * and throttled intervals take no samples that any loss count reports.
 * The kernel's per-tick budget (rate / HZ) makes this an approximation. */
struct pf_period_floor
{
    bool known; /* both values were readable */
    uint64_t max_sample_rate; /* samples per second */
    uint64_t cpu_max_khz; /* cpu0 cpuinfo_max_freq */
    uint64_t floor_cycles;
};

struct pf_sampling_report
{
    struct pf_request_summary request;
    bool workload_ran;
    uint64_t checksum; /* meaningful only when workload_ran */
    const struct pf_sampling_config *config; /* borrowed for the call */
    const struct pf_sampling_result *result; /* borrowed for the call */
    const struct pf_period_floor *floor; /* borrowed for the call */
};

/* {"kind":"sampling",...}: status, ring statistics and loss accounting.
 * A loss quantity that is not known is written as null, never as 0. */
void pf_report_sampling(FILE *stream, const struct pf_sampling_report *report);

/* {"kind":"sample",...}: one retained sample. */
void pf_report_sample(FILE *stream, const struct pf_sample *sample);

/* {"kind":"mapping","line":...}: one /proc/self/maps line, JSON-escaped;
 * the trailing newline is dropped and other control bytes are omitted. */
void pf_report_mapping(FILE *stream, const char *line);

/* {"kind":"family",...}: whether this build contains `family`. */
void pf_report_family(FILE *stream, enum pf_family family);

/* {"kind":"check",...}: a request validated without opening perf events. */
void pf_report_check(FILE *stream, const struct pf_request_summary *request,
                     uint64_t checksum,
                     const struct pf_sampling_config *sampling,
                     const struct pf_period_floor *floor);

#endif /* PF_PMU_REPORT_H */
