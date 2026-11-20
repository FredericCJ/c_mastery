/*
 * Concurrent-state dossier workload (WORKED_EXAMPLE.md). Supplied course
 * infrastructure, derived from the concurrency family of pmu_workloads.c,
 * not a learner exercise.
 *
 * Two threads, each pinned to one CPU of the allowed set, execute the same
 * relaxed atomic increment loop on a slot chosen by the layout:
 *
 *   shared    both threads update one slot (true sharing)
 *   adjacent  distinct slots 8 bytes apart in one 64-byte line
 *   separate  distinct slots 128 bytes apart (per-thread state)
 *   solo      the peer is created, placed and joined but never increments
 *
 * Only the slot addresses differ between layouts: every thread runs the same
 * machine code. 64 bytes is the coherency_line_size this host reports; the
 * layouts are explicit address interventions, not a universal line size.
 *
 * Lifecycle, with every phase outside the measured window except the loop:
 *
 *   setup     allocate the page-aligned arena, pin the caller, create the
 *             peer already placed on its CPU, open each thread's own group
 *   warm-up   `warmup` increments on each active thread, then a spinning
 *             barrier, so that both windows start together
 *   window    each thread starts its group, reads the clock, performs
 *             `iterations` increments, reads the clock, stops its group
 *   tail      a thread that finished keeps incrementing (uncounted) until
 *             its peer has finished, so each window is contended throughout
 *   teardown  join, restore the caller's affinity, check the contract
 *
 * Correctness contract, checked after every run (cs_contract_holds):
 *   shared    slot == 2 * (warmup + iterations) + tail[0] + tail[1]
 *   adjacent, separate
 *             slot[t] == warmup + iterations + tail[t] for each thread t
 *   solo      slot[0] == warmup + iterations, slot[1] == 0, no tail
 * and the addresses realise the layout: one slot (shared), one 64-byte line
 * with two slots (adjacent), different 128-byte blocks (separate, solo).
 */
#ifndef CS_CONCURRENT_STATE_H
#define CS_CONCURRENT_STATE_H

#include "dossier_common.h"

#include <stdbool.h>
#include <stdint.h>

#define CS_LINE_BYTES 64u /* the line the layouts are built around */
#define CS_SEPARATION_BYTES 128u /* distance of the separate slot */
#define CS_MAX_ITERATIONS (UINT64_C(1) << 40)

enum cs_layout
{
    CS_LAYOUT_SHARED = 0,
    CS_LAYOUT_ADJACENT,
    CS_LAYOUT_SEPARATE,
    CS_LAYOUT_SOLO,
    CS_LAYOUT_COUNT
};

/* Exact layout name to value; false, leaving *layout unchanged, otherwise. */
bool cs_layout_lookup(const char *name, enum cs_layout *layout);
const char *cs_layout_name(enum cs_layout layout);

enum cs_status
{
    CS_OK = 0,
    CS_INVALID, /* layout, counts or event list out of range */
    CS_PLACEMENT, /* a CPU outside the allowed set, or pinning failed */
    CS_NO_MEMORY,
    CS_THREAD, /* barrier or thread creation/join failed */
    CS_CLOCK, /* a clock read failed */
    CS_CONTRACT /* the final state broke the correctness contract */
};

const char *cs_status_name(enum cs_status status);

struct cs_request
{
    enum cs_layout layout;
    unsigned cpu[2]; /* [0] measured (calling) thread, [1] peer */
    uint64_t iterations; /* per thread inside the window, 1..MAX */
    uint64_t warmup; /* per thread before the window, 0..MAX */
    const struct pfd_events *events; /* NULL or empty: nothing counted */
};

struct cs_thread_result
{
    struct pfd_window window;
    uint64_t measured; /* increments inside the window */
    uint64_t tail; /* increments after the window */
    uint64_t tail_end_ns; /* when the tail stopped (outside the window) */
    uint64_t final_value; /* the thread's slot after both joined */
    uintptr_t slot_address;
    bool active; /* false for the solo peer */
};

struct cs_result
{
    enum cs_status status;
    struct cs_thread_result thread[2];
    /* Lifecycle timestamps (CLOCK_MONOTONIC_RAW, ns), all outside the
     * windows: before pthread_create, the peer's first statement, after
     * pthread_create returned, after pthread_join returned. */
    uint64_t create_begin_ns;
    uint64_t peer_entry_ns;
    uint64_t create_return_ns;
    uint64_t join_return_ns;
};

/* Run one complete lifecycle. On CS_OK every phase completed and the
 * contract holds; the windows' group states still say whether counting
 * succeeded. Any other status leaves no thread running and no memory
 * held. Blocks for the duration of the run. */
enum cs_status cs_run(const struct cs_request *request,
                      struct cs_result *result);

/* The contract above, evaluated on a completed result. */
bool cs_contract_holds(const struct cs_request *request,
                       const struct cs_result *result);

/* The measured loop: `iterations` relaxed increments of *slot. External so
 * the generated code can be inspected by name. */
void cs_increment(_Atomic uint64_t *slot, uint64_t iterations);

#endif /* CS_CONCURRENT_STATE_H */
