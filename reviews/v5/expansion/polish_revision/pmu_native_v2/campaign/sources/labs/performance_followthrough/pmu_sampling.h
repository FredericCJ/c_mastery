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

/* Record kinds the supplied consumer counts itself. The parser contract
 * (pmu.h) counts them only as unknown, yet they change what a sampled
 * observation means: between THROTTLE and UNTHROTTLE no sample was taken and
 * no loss count records the gap. */
struct pf_ring_kinds
{
    uint64_t throttle; /* PERF_RECORD_THROTTLE (5) */
    uint64_t unthrottle; /* PERF_RECORD_UNTHROTTLE (6) */
};

/* Count the kinds above in the consumed byte range [from, to) of a ring of
 * `capacity` bytes (a power of two). Reads headers with wraparound; stops at
 * the first malformed header or if the range exceeds the capacity. Pure. */
void pf_ring_count_kinds(const unsigned char *data, size_t capacity,
                         uint64_t from, uint64_t to,
                         struct pf_ring_kinds *kinds);

/* Consume every record published so far with pf_ring_drain, count the kinds
 * of the consumed records (kinds may be NULL), then publish the new tail.
 * Returns the pf_ring_drain status; on failure the tail stays at the last
 * completely consumed record. */
int pf_ring_view_drain(struct pf_ring_view *view, size_t budget,
                       struct pf_ring_stats *stats, struct pf_ring_kinds *kinds,
                       pf_sample_sink_fn sink, void *context);

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

/* Where a sampling run stopped. Only PF_SAMPLING_PARSER is a failure of the
 * learner's component (pf_ring_drain); every other stage is the host or the
 * supplied adapter. */
enum pf_sampling_stage
{
    PF_SAMPLING_COMPLETE = 0, /* no failure */
    PF_SAMPLING_CONFIG, /* page size or data pages unusable */
    PF_SAMPLING_OPEN, /* perf_event_open refused (status says why) */
    PF_SAMPLING_MAP, /* mmap of the ring failed */
    PF_SAMPLING_LAYOUT, /* the kernel's ring description failed validation */
    PF_SAMPLING_THREAD, /* the drainer thread could not be started */
    PF_SAMPLING_CONTROL, /* reset, enable or disable failed */
    PF_SAMPLING_PARSER, /* pf_ring_drain stopped on the kernel's records */
    PF_SAMPLING_READ /* the count read() failed or was malformed */
};

struct pf_sampling_result
{
    const char *status; /* measured, denied, unavailable, map-failed,
                           invalid, never-scheduled or no-samples */
    enum pf_sampling_stage stage;
    int open_errno;
    int control_errno; /* errno of a failed reset, enable or disable */
    int cleanup_errno;
    bool parser_ran; /* parser_status is meaningful */
    int parser_status;
    struct pf_ring_stats ring;
    struct pf_ring_kinds kinds;
    struct pf_sample_reading reading;
    enum pf_lost_support lost_support;
    struct pf_loss_account loss;
    uint64_t drains;
    struct pf_sample_buffer samples;
};

/* Receives the caller's context unchanged; runs on the calling thread. */
typedef void (*pf_workload_fn)(void *);

/* Starting and stopping the sampled interval. The Linux adapter resets and
 * enables, then disables, the perf event; tests drive a model producer.
 * Each returns 0, or -1 with errno set. */
struct pf_sampling_control
{
    void *context;
    int (*start)(void *context);
    int (*stop)(void *context);
};

/* The drain lifecycle shared by the Linux adapter and its tests: start, run
 * `workload(context)` on the calling thread, stop, and drain `view` as
 * config->drain requires. In concurrent mode a drainer thread waits on
 * `poll_fd` (at most config->poll_interval_ms between passes) and owns
 * data_tail until it is joined; the final drain then runs on the calling
 * thread unless the drainer's parser had already failed. Fills the ring,
 * kinds, sample, drain, parser and control_errno fields of *result and
 * returns the first failed stage (PF_SAMPLING_COMPLETE when none). Joining
 * the drainer cannot fail in a sound program; if it does, the drainer would
 * outlive the ring and this call's state, so the adapter aborts instead of
 * returning. */
enum pf_sampling_stage pf_sampling_observe(
    const struct pf_sampling_config *config, const struct pf_ring_view *view,
    int poll_fd, const struct pf_sampling_control *control,
    pf_workload_fn workload, void *context, struct pf_sampling_result *result);

/* Run `workload(context)` on the calling thread with cycle sampling enabled
 * for that thread's user-space execution only. Before the call,
 * `result->samples.items` must point to caller-owned storage for
 * `config->sample_capacity` samples (it may be NULL when the capacity is 0).
 * Blocks until the workload returns and, in concurrent mode, the drainer
 * thread has been joined. Every acquired descriptor and mapping is released
 * before return. Failures are reported in *result (see pf_sampling_observe
 * for the one unrecoverable case). Loss is `lossless` only for a `measured`
 * observation (pmu_loss.h). */
void pf_sampling_run(const struct pf_sampling_config *config,
                     pf_workload_fn workload, void *context,
                     struct pf_sampling_result *result);

const char *pf_lost_support_name(enum pf_lost_support support);
const char *pf_drain_mode_name(enum pf_drain_mode mode);
const char *pf_sampling_stage_name(enum pf_sampling_stage stage);

#endif /* PF_PMU_SAMPLING_H */
