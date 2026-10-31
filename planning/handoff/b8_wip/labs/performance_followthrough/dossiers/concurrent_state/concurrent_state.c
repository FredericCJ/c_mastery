/*
 * Concurrent-state workload. Contract and lifecycle: concurrent_state.h.
 */
#define _GNU_SOURCE /* pthread_getaffinity_np, pthread_setaffinity_np */
#include "concurrent_state.h"

#include <errno.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#define CS_ARENA_BYTES 4096u

/* One flag per 128-byte block, away from every counter slot. */
struct cs_flag
{
    _Alignas(CS_SEPARATION_BYTES) atomic_uint value;
};

/* The page-aligned arena. near[0] and near[1] share one 64-byte line;
 * far starts 128 bytes after near[0]. No other object shares the page. */
struct cs_arena
{
    _Alignas(CS_SEPARATION_BYTES) _Atomic uint64_t near[2];
    _Alignas(CS_SEPARATION_BYTES) _Atomic uint64_t far;
    struct cs_flag done[2];
    struct cs_flag arrived; /* the spinning start barrier */
};

_Static_assert(sizeof(struct cs_arena) <= CS_ARENA_BYTES,
               "the arena must fit in its page");

/* Everything both threads read; written only before the peer starts. */
struct cs_shared
{
    pthread_barrier_t ready; /* both groups open, both placed */
    atomic_uint *arrived; /* both warmed up: the spinning start barrier */
    const struct cs_request *request;
    const struct pfd_allowed *allowed; /* captured before any pinning */
};

struct cs_thread
{
    struct cs_shared *shared;
    _Atomic uint64_t *slot;
    atomic_uint *own_done;
    atomic_uint *peer_done;
    struct cs_thread_result *result;
    uint64_t entry_ns;
    bool entry_clock_ok;
    bool tail_clock_ok;
    bool barrier_ok;
};

static const char *const layout_names[CS_LAYOUT_COUNT] = {
    [CS_LAYOUT_SHARED] = "shared",
    [CS_LAYOUT_ADJACENT] = "adjacent",
    [CS_LAYOUT_SEPARATE] = "separate",
    [CS_LAYOUT_SOLO] = "solo",
};

bool cs_layout_lookup(const char *name, enum cs_layout *layout)
{
    for (unsigned i = 0; (name != NULL) && (i < CS_LAYOUT_COUNT); ++i)
    {
        if (strcmp(name, layout_names[i]) == 0)
        {
            *layout = (enum cs_layout)i;
            return true;
        }
    }
    return false;
}

const char *cs_layout_name(enum cs_layout layout)
{
    return ((unsigned)layout < CS_LAYOUT_COUNT) ? layout_names[layout]
                                                : "invalid";
}

const char *cs_status_name(enum cs_status status)
{
    switch (status)
    {
        case CS_OK:
            return "ok";
        case CS_INVALID:
            return "invalid";
        case CS_PLACEMENT:
            return "placement";
        case CS_NO_MEMORY:
            return "no-memory";
        case CS_THREAD:
            return "thread";
        case CS_CLOCK:
            return "clock";
        case CS_CONTRACT:
            return "contract";
        default:
            return "invalid-status";
    }
}

void cs_increment(_Atomic uint64_t *slot, uint64_t iterations)
{
    for (uint64_t k = 0; k < iterations; ++k)
    {
        atomic_fetch_add_explicit(slot, 1u, memory_order_relaxed);
    }
}

/* Keep contending until the peer has closed its window; return how many
 * increments that took. The acquire pairs with the peer's release store,
 * which follows its last window increment. */
static uint64_t contend_until_done(_Atomic uint64_t *slot,
                                   const atomic_uint *peer_done)
{
    uint64_t tail = 0;

    while (atomic_load_explicit(peer_done, memory_order_acquire) == 0u)
    {
        atomic_fetch_add_explicit(slot, 1u, memory_order_relaxed);
        ++tail;
    }
    return tail;
}

static bool wait_at(pthread_barrier_t *barrier)
{
    int status = pthread_barrier_wait(barrier);

    return (status == 0) || (status == PTHREAD_BARRIER_SERIAL_THREAD);
}

/* One-shot barrier for the window start. A sleeping barrier wakes the
 * first arrival through the scheduler, which was observed to start its
 * window up to milliseconds after its peer's; spinning keeps both threads
 * running so they leave together. The occasional sched_yield lets two
 * threads that share one CPU make progress. */
static void spin_until_both_arrived(atomic_uint *arrived)
{
    unsigned spins = 0;

    (void)atomic_fetch_add_explicit(arrived, 1u, memory_order_acq_rel);
    while (atomic_load_explicit(arrived, memory_order_acquire) < 2u)
    {
        if ((++spins % 4096u) == 0u)
        {
            (void)sched_yield();
        }
    }
}

/* The body both threads run: the caller directly, the peer as its start
 * routine. Barriers are always passed, even after a failure, so neither
 * thread can be left waiting for the other. */
static void *run_thread(void *context)
{
    struct cs_thread *thread = context;
    const struct cs_request *request = thread->shared->request;
    struct cs_thread_result *result = thread->result;
    bool active = result->active;

    thread->entry_clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &thread->entry_ns);
    (void)pfd_window_open(&result->window, active ? request->events : NULL);
    thread->barrier_ok = wait_at(&thread->shared->ready);
    if (active)
    {
        cs_increment(thread->slot, request->warmup);
    }
    spin_until_both_arrived(thread->shared->arrived);
    if (active)
    {
        pfd_window_begin(&result->window);
        cs_increment(thread->slot, request->iterations);
        pfd_window_end(&result->window);
        result->measured = request->iterations;
    }
    atomic_store_explicit(thread->own_done, 1u, memory_order_release);
    if (active && (request->layout != CS_LAYOUT_SOLO))
    {
        result->tail = contend_until_done(thread->slot, thread->peer_done);
    }
    thread->tail_clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &result->tail_end_ns);
    pfd_window_close(&result->window);
    return NULL;
}

static enum cs_status validate(const struct cs_request *request,
                               const struct pfd_allowed *allowed)
{
    if (((unsigned)request->layout >= CS_LAYOUT_COUNT) ||
        (request->iterations == 0u) ||
        (request->iterations > CS_MAX_ITERATIONS) ||
        (request->warmup > CS_MAX_ITERATIONS) ||
        ((request->events != NULL) &&
         (request->events->count > PFD_MAX_EVENTS)))
    {
        return CS_INVALID;
    }
    if (!pfd_allowed_has(allowed, request->cpu[0]) ||
        !pfd_allowed_has(allowed, request->cpu[1]))
    {
        return CS_PLACEMENT;
    }
    return CS_OK;
}

/* Bind each thread to its slot and flags as the layout prescribes. */
static void assign_slots(struct cs_arena *arena, enum cs_layout layout,
                         struct cs_thread threads[2])
{
    threads[0].slot = &arena->near[0];
    switch (layout)
    {
        case CS_LAYOUT_SHARED:
            threads[1].slot = &arena->near[0];
            break;
        case CS_LAYOUT_ADJACENT:
            threads[1].slot = &arena->near[1];
            break;
        case CS_LAYOUT_SEPARATE:
        case CS_LAYOUT_SOLO:
        default:
            threads[1].slot = &arena->far;
            break;
    }
    for (unsigned i = 0; i < 2u; ++i)
    {
        threads[i].own_done = &arena->done[i].value;
        threads[i].peer_done = &arena->done[1u - i].value;
        threads[i].result->slot_address = (uintptr_t)threads[i].slot;
    }
}

static struct cs_arena *arena_create(void)
{
    struct cs_arena *arena = aligned_alloc(CS_ARENA_BYTES, CS_ARENA_BYTES);

    if (arena == NULL)
    {
        return NULL;
    }
    memset(arena, 0, CS_ARENA_BYTES);
    atomic_init(&arena->near[0], 0u);
    atomic_init(&arena->near[1], 0u);
    atomic_init(&arena->far, 0u);
    atomic_init(&arena->done[0].value, 0u);
    atomic_init(&arena->done[1].value, 0u);
    atomic_init(&arena->arrived.value, 0u);
    return arena;
}

/* Create the placed peer, run the caller's share, join. The barriers and
 * the attribute are owned by the caller of this function. */
static enum cs_status run_pair(struct cs_shared *shared,
                               struct cs_thread threads[2],
                               struct cs_result *result)
{
    pthread_attr_t attr;
    pthread_t peer;
    bool clock_ok = true;

    if (pthread_attr_init(&attr) != 0)
    {
        return CS_THREAD;
    }
    if (pfd_place_attr(shared->allowed, &attr, shared->request->cpu[1]) != 0)
    {
        (void)pthread_attr_destroy(&attr);
        return CS_PLACEMENT;
    }
    clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &result->create_begin_ns);
    int created = pthread_create(&peer, &attr, run_thread, &threads[1]);
    clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &result->create_return_ns) &&
        clock_ok;
    (void)pthread_attr_destroy(&attr);
    if (created != 0)
    {
        return CS_THREAD;
    }
    (void)run_thread(&threads[0]);
    int joined = pthread_join(peer, NULL);
    clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &result->join_return_ns) &&
        clock_ok;
    result->peer_entry_ns = threads[1].entry_ns;
    if ((joined != 0) || !threads[0].barrier_ok || !threads[1].barrier_ok)
    {
        return CS_THREAD;
    }
    bool windows_ok =
        !threads[0].result->active || threads[0].result->window.clock_ok;
    windows_ok = windows_ok && (!threads[1].result->active ||
                                threads[1].result->window.clock_ok);
    if (!clock_ok || !threads[0].entry_clock_ok || !threads[1].entry_clock_ok ||
        !threads[0].tail_clock_ok || !threads[1].tail_clock_ok || !windows_ok)
    {
        return CS_CLOCK;
    }
    return CS_OK;
}

/* Pin the caller, initialize both barriers and run the pair. On return the
 * caller's own affinity is restored and both barriers are destroyed. */
static enum cs_status run_placed(const struct cs_request *request,
                                 const struct pfd_allowed *allowed,
                                 struct cs_arena *arena,
                                 struct cs_result *result)
{
    struct cs_shared shared = {.request = request,
                               .allowed = allowed,
                               .arrived = &arena->arrived.value};
    struct cs_thread threads[2] = {
        {.shared = &shared, .result = &result->thread[0]},
        {.shared = &shared, .result = &result->thread[1]},
    };
    cpu_set_t original;
    enum cs_status status = CS_THREAD;

    result->thread[0].active = true;
    result->thread[1].active = (request->layout != CS_LAYOUT_SOLO);
    assign_slots(arena, request->layout, threads);
    CPU_ZERO(&original);
    if (pthread_getaffinity_np(pthread_self(), sizeof original, &original) != 0)
    {
        return CS_PLACEMENT;
    }
    if (pfd_pin_calling_thread(allowed, request->cpu[0]) != 0)
    {
        return CS_PLACEMENT;
    }
    if (pthread_barrier_init(&shared.ready, NULL, 2) == 0)
    {
        status = run_pair(&shared, threads, result);
        (void)pthread_barrier_destroy(&shared.ready);
    }
    if (pthread_setaffinity_np(pthread_self(), sizeof original, &original) !=
            0 &&
        (status == CS_OK))
    {
        status = CS_PLACEMENT;
    }
    for (unsigned i = 0; i < 2u; ++i)
    {
        result->thread[i].final_value =
            atomic_load_explicit(threads[i].slot, memory_order_relaxed);
    }
    return status;
}

enum cs_status cs_run(const struct cs_request *request,
                      struct cs_result *result)
{
    if ((request == NULL) || (result == NULL))
    {
        return CS_INVALID;
    }
    memset(result, 0, sizeof *result);
    /* The caller's affinity now is the allowed set: cs_run restores it
     * before returning, so every run starts from the same snapshot. */
    struct pfd_allowed allowed;
    if (!pfd_allowed_capture(&allowed))
    {
        result->status = CS_PLACEMENT;
        return result->status;
    }
    result->status = validate(request, &allowed);
    if (result->status != CS_OK)
    {
        return result->status;
    }
    struct cs_arena *arena = arena_create();
    if (arena == NULL)
    {
        result->status = CS_NO_MEMORY;
        return result->status;
    }
    result->status = run_placed(request, &allowed, arena, result);
    free(arena);
    if ((result->status == CS_OK) && !cs_contract_holds(request, result))
    {
        result->status = CS_CONTRACT;
    }
    return result->status;
}

bool cs_contract_holds(const struct cs_request *request,
                       const struct cs_result *result)
{
    const struct cs_thread_result *t = result->thread;
    uint64_t per_thread = request->warmup + request->iterations;
    uintptr_t a = t[0].slot_address;
    uintptr_t b = t[1].slot_address;
    bool distinct_blocks =
        (a / CS_SEPARATION_BYTES) != (b / CS_SEPARATION_BYTES);

    if (!t[0].active || (t[0].measured != request->iterations))
    {
        return false;
    }
    switch (request->layout)
    {
        case CS_LAYOUT_SHARED:
            return (a == b) && t[1].active &&
                   (t[1].measured == request->iterations) &&
                   (t[0].final_value == t[1].final_value) &&
                   (t[0].final_value ==
                    2u * per_thread + t[0].tail + t[1].tail);
        case CS_LAYOUT_ADJACENT:
        case CS_LAYOUT_SEPARATE:
        {
            bool placed =
                (request->layout == CS_LAYOUT_ADJACENT)
                    ? ((a != b) && ((a / CS_LINE_BYTES) == (b / CS_LINE_BYTES)))
                    : distinct_blocks;

            return placed && t[1].active &&
                   (t[1].measured == request->iterations) &&
                   (t[0].final_value == per_thread + t[0].tail) &&
                   (t[1].final_value == per_thread + t[1].tail);
        }
        case CS_LAYOUT_SOLO:
            return distinct_blocks && !t[1].active && (t[1].measured == 0u) &&
                   (t[0].tail == 0u) && (t[1].tail == 0u) &&
                   (t[0].final_value == per_thread) && (t[1].final_value == 0u);
        default:
            return false;
    }
}
