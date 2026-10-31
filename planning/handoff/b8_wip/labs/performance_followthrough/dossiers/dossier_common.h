/*
 * Shared support for the diagnostic dossier drivers (dossiers/README.md).
 * Supplied course infrastructure, not a learner exercise.
 *
 * A driver measures one window on one or more threads. Each measuring
 * thread owns its own strict counter group (pmu.h: the calling thread only,
 * on any CPU, user space only), opened on that thread after it has been
 * placed on its CPU, so the counts are that thread's alone. A window is:
 * start the group, note the CPU, read the clock, run the work, read the
 * clock, note the CPU, stop the group. The two clock reads and CPU queries
 * lie inside the counted interval; they are a small constant per window.
 *
 * Placement only narrows: a thread is pinned to one CPU of the process's
 * allowed set (sched_getaffinity). Nothing here changes a system policy.
 *
 * Linux/glibc dependencies: pthread_setaffinity_np,
 * pthread_attr_setaffinity_np, sched_getaffinity and sched_getcpu (GNU
 * extensions, hence _GNU_SOURCE before any include in the .c files), the
 * perf ABI through pmu_linux.c, and labs/support (cm_parse, cm_clock).
 */
#ifndef PFD_DOSSIER_COMMON_H
#define PFD_DOSSIER_COMMON_H

#include "cm_clock.h"
#include "pmu.h"

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PFD_MAX_EVENTS PF_PMU_MAX
#define PFD_MAX_CPU 1023u

enum pfd_exit
{
    PFD_EXIT_OK = 0,
    PFD_EXIT_CONTRACT = 1, /* a variant broke its correctness contract */
    PFD_EXIT_INVALID = 2, /* invalid request or infrastructure failure */
};

/* Raw events of the core PMU (perf_event_attr.type PERF_TYPE_RAW). The
 * runner resolves event names to configurations from `perf list` and the
 * PMU's sysfs format; the driver never interprets a configuration. */
struct pfd_events
{
    size_t count;
    struct pf_pmu_event events[PFD_MAX_EVENTS];
};

/* Parse `count` plain-decimal raw configurations (cm_parse.h). Returns
 * false, after a diagnostic on stderr, for any rejected text or for more
 * than PFD_MAX_EVENTS configurations. count 0 is valid: no counting. */
bool pfd_parse_events(char *const *texts, size_t count,
                      struct pfd_events *events);

/* Checked decimal arguments: false, after a diagnostic naming `name`, for
 * any text cm_parse rejects or a value outside [min, max]. */
bool pfd_parse_unsigned(const char *name, const char *text, unsigned min,
                        unsigned max, unsigned *value);
bool pfd_parse_u64(const char *name, const char *text, uint64_t min,
                   uint64_t max, uint64_t *value);

/* A snapshot of the CPUs a thread may run on. sched_getaffinity reports
 * the calling thread's own mask, which shrinks once that thread is pinned,
 * so a driver captures the snapshot before it pins anything and checks
 * every placement against it. */
struct pfd_allowed
{
    uint64_t bits[(PFD_MAX_CPU + 1u) / 64u];
};

/* Capture the calling thread's current affinity. Returns false when it
 * cannot be read; *allowed is then empty. */
bool pfd_allowed_capture(struct pfd_allowed *allowed);

/* True when `cpu` is in the snapshot. */
bool pfd_allowed_has(const struct pfd_allowed *allowed, unsigned cpu);

/* Pin the calling thread to `cpu`. Returns 0, EINVAL when `cpu` is not in
 * `allowed` (nothing is changed), or the pthread_setaffinity_np error.
 * Never widens the allowed set. */
int pfd_pin_calling_thread(const struct pfd_allowed *allowed, unsigned cpu);

/* Make threads created with `attr` start on `cpu`, under the same rule. */
int pfd_place_attr(const struct pfd_allowed *allowed, pthread_attr_t *attr,
                   unsigned cpu);

/* One measured window on the calling thread. The thread's context-switch
 * counts (getrusage RUSAGE_THREAD) are read just outside the counted
 * interval: an involuntary switch means the thread was preempted inside
 * its window, and the elapsed time then includes another task's work. */
struct pfd_window
{
    struct pf_pmu group;
    const struct pfd_events *events;
    enum pf_pmu_state state; /* PF_PMU_EMPTY when nothing was requested */
    bool clock_ok;
    bool switches_ok;
    uint64_t start_ns;
    uint64_t end_ns;
    int cpu_start;
    int cpu_end;
    long involuntary_before;
    long involuntary_after;
    long voluntary_before;
    long voluntary_after;
};

/* Prepare `window` and, when `events` requests any, open the strict group
 * for the calling thread. Must run on the thread that will be measured.
 * Returns the group state (PF_PMU_READY, a refusal, or PF_PMU_EMPTY). */
enum pf_pmu_state pfd_window_open(struct pfd_window *window,
                                  const struct pfd_events *events);

/* Read the switch counts, start the group when it is ready, then note the
 * CPU and the clock. */
void pfd_window_begin(struct pfd_window *window);

/* Read the clock and the CPU, stop and read the group, then read the
 * switch counts. */
void pfd_window_end(struct pfd_window *window);

/* Close the group's descriptors (idempotent). */
void pfd_window_close(struct pfd_window *window);

/* The window's elapsed time, or 0 when the clock failed or reversed. */
uint64_t pfd_window_elapsed_ns(const struct pfd_window *window);

/* JSON fragments for one output line. Each prints `"key":value` without a
 * leading or trailing comma. */
void pfd_print_window(FILE *out, const char *key,
                      const struct pfd_window *window);
void pfd_print_clock(FILE *out, const struct cm_clock_calibration *clock);
void pfd_print_u64_array(FILE *out, const char *key, const uint64_t *values,
                         size_t count);

#endif /* PFD_DOSSIER_COMMON_H */
