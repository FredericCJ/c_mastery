/*
 * Supplied adapter: hardware-cycle sampling into a perf mmap ring, with final
 * or concurrent draining and complete loss accounting (pmu_loss.h).
 *
 * This is course infrastructure, not a learner exercise. It depends on the
 * Linux perf_event ABI, POSIX threads and C11 atomics; the record parser it
 * calls (pf_ring_drain, pmu.h) is the learner's component.
 *
 * Memory-ordering contract of the mmap ring (Linux perf ring buffer
 * documentation): the kernel publishes records, then data_head. The consumer
 * loads data_head with acquire ordering, reads the published bytes, then
 * stores data_tail with release ordering so the kernel cannot overwrite bytes
 * that are still being read. Only one consumer may own data_tail: the
 * concurrent drainer owns it until it is joined, then the caller performs
 * the final drain.
 */
#ifndef PF_PMU_SAMPLING_H
#define PF_PMU_SAMPLING_H

#include "pmu.h"
#include "pmu_loss.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A consumer's view of a ring. In the real collector `head` and `tail`
 * point at data_head and data_tail in the kernel's metadata page; tests
 * point them at a model. Those fields are plain 64-bit integers shared with
 * the kernel, so they are accessed with the GCC/Clang __atomic builtins (an
 * explicit compiler extension) rather than declared as ISO C _Atomic objects. */
struct pf_ring_view
{
    const unsigned char *data;
    size_t capacity; /* bytes; a power of two */
    uint64_t *head;
    uint64_t *tail;
};

/* Receives (context, sample): the caller's context unchanged and a sample
 * borrowed only until the call returns. */
typedef void (*pf_sample_sink_fn)(void *, const struct pf_sample *);

/* Consume every record published so far, then publish the new tail.
 * Returns the pf_ring_drain status; on failure the tail stays at the last
 * completely consumed record. */
int pf_ring_view_drain(struct pf_ring_view *view, size_t budget,
                       struct pf_ring_stats *stats, pf_sample_sink_fn sink,
                       void *context);

enum pf_drain_mode
{
    PF_DRAIN_FINAL = 0, /* drain once, after the event is disabled */
    PF_DRAIN_CONCURRENT /* a drainer thread drains while the workload runs */
};

enum pf_lost_support
{
    PF_LOST_SUPPORTED = 0, /* PERF_FORMAT_LOST requested and accepted */
    PF_LOST_NOT_REQUESTED,
    PF_LOST_HEADER_MISSING, /* built against headers without the flag */
    PF_LOST_KERNEL_REJECTED, /* the kernel refused the read format (EINVAL) */
    PF_LOST_UNDETERMINED /* the event itself could not be opened */
};

struct pf_sampling_config
{
    uint64_t sample_period_cycles;
    unsigned data_pages; /* a power of two, at least 1 */
    unsigned precise_ip; /* 0..3 */
    enum pf_drain_mode drain;
    unsigned poll_interval_ms; /* concurrent drainer wake-up bound */
    bool request_lost_count;
    size_t sample_capacity; /* samples kept for the report (bounded memory);
                               may be 0 to count samples without keeping any */
};

/* Samples retained by the collector. Retention overflow is the collector's
 * own bound, reported separately from any kernel loss. */
struct pf_sample_buffer
{
    struct pf_sample *items;
    size_t capacity;
    size_t count;
    uint64_t not_retained;
};

struct pf_sampling_result
{
    const char *status; /* measured, denied, unavailable, map-failed,
                           invalid, never-scheduled or no-samples */
    int open_errno;
    int cleanup_errno;
    int parser_status;
    struct pf_ring_stats ring;
    struct pf_sample_reading reading;
    enum pf_lost_support lost_support;
    struct pf_loss_account loss;
    uint64_t drains;
    struct pf_sample_buffer samples;
};

/* Receives the caller's context unchanged; runs on the calling thread. */
typedef void (*pf_workload_fn)(void *);

/* Run `workload(context)` on the calling thread with cycle sampling enabled
 * for that thread's user-space execution only. Before the call,
 * `result->samples.items` must point to caller-owned storage for
 * `config->sample_capacity` samples (it may be NULL when the capacity is 0).
 * Blocks until the workload returns and, in concurrent mode, the drainer
 * thread has been joined. Every acquired descriptor and mapping is released
 * before return; failures are reported in *result, never by aborting. */
void pf_sampling_run(const struct pf_sampling_config *config,
                     pf_workload_fn workload, void *context,
                     struct pf_sampling_result *result);

const char *pf_lost_support_name(enum pf_lost_support support);
const char *pf_drain_mode_name(enum pf_drain_mode mode);

#endif /* PF_PMU_SAMPLING_H */
