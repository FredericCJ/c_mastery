#define _POSIX_C_SOURCE 200809L
#include "allocation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Prints one JSON line per measurement: 2 policies x 2 instrumentation modes
 * x 3 workloads x 9 samples = 108 lines. Each sample times 128 repetitions of
 * "fill the heap with 8 objects, free every other one, then ask for 60 bytes"
 * (which fails when the free holes are too small). Any unexpected allocator
 * status aborts: a benchmark of a broken allocator is meaningless.
 */
enum
{
    REPETITIONS = 128,
    OBJECTS = 8,
    SAMPLES = 9,
    WORKLOADS = 3,
    SMALL_REQUEST = 4,
    LARGE_REQUEST = 20,
    PROBE_REQUEST = 60
};

static const uint64_t NS_PER_SECOND = UINT64_C(1000000000);

static uint64_t now_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * NS_PER_SECOND + (uint64_t)t.tv_nsec;
}

/* Instrumentation adds a full invariant check after every mutation. */
static void check_if_instrumented(const struct al_heap *h, unsigned on)
{
    if (on && al_heap_check(h))
    {
        abort();
    }
}

/*
 * Object sizes alternate large/small. Workload 0 and 1 allocate large first;
 * workload 2 allocates the sizes in reverse order, so the objects freed by
 * free_alternate are the small ones. In workloads 0 and 2 the freed blocks
 * are holes separated by live neighbours. Workload 1 frees the contiguous run
 * of handles 0..3, which merges into one free block.
 */
static void fill_heap(struct al_heap *h, struct al_handle handles[OBJECTS],
                      unsigned workload, unsigned instrumentation)
{
    for (unsigned i = 0; i < OBJECTS; ++i)
    {
        unsigned index = workload == 2 ? OBJECTS - 1 - i : i;
        size_t n = index % 2 ? SMALL_REQUEST : LARGE_REQUEST;
        if (al_heap_take(h, n, 8, &handles[i]))
        {
            abort();
        }
        check_if_instrumented(h, instrumentation);
    }
}

/* Frees handles 0, 2, 4, 6 (workload 1: handles 0, 1, 2, 3). */
static void free_alternate(struct al_heap *h,
                           const struct al_handle handles[OBJECTS],
                           unsigned workload, unsigned instrumentation)
{
    for (unsigned i = 0; i < OBJECTS; i += 2)
    {
        unsigned index = workload == 1 ? i / 2 : i;
        if (al_heap_release(h, handles[index]))
        {
            abort();
        }
        check_if_instrumented(h, instrumentation);
    }
}

struct sample
{
    uint64_t elapsed_ns;
    size_t peak_payload;
    size_t peak_charged;
    size_t total_free;
    size_t largest_free;
    unsigned failures; /* probe requests that returned AL_FULL */
    struct al_heap last; /* heap after the final repetition */
};

static void run_sample(unsigned policy, unsigned instrumentation,
                       unsigned workload, struct sample *out)
{
    struct al_handle handles[OBJECTS];
    *out = (struct sample){0};
    uint64_t start = now_ns();
    for (unsigned repeat = 0; repeat < REPETITIONS; ++repeat)
    {
        struct al_heap *h = &out->last;
        if (al_heap_init(h, AL_HEAP_BYTES, policy, 0, 1))
        {
            abort();
        }
        fill_heap(h, handles, workload, instrumentation);
        free_alternate(h, handles, workload, instrumentation);
        struct al_handle request;
        int status = al_heap_take(h, PROBE_REQUEST, 8, &request);
        if (status == AL_FULL)
        {
            ++out->failures;
        }
        else if (status != AL_OK)
        {
            abort();
        }
        if (al_heap_stats(h, &out->total_free, &out->largest_free))
        {
            abort();
        }
        out->peak_payload = h->peak_payload;
        out->peak_charged = h->peak_charged;
    }
    out->elapsed_ns = now_ns() - start;
}

static void print_sample(unsigned policy, unsigned instrumentation,
                         unsigned workload, unsigned index,
                         const struct sample *s)
{
    printf("{\"policy\":%u,\"extra_invariant_instrumentation\":%u,"
           "\"workload\":%u,\"sample\":%u,\"repetitions\":128,\"ns\":%llu,"
           "\"peak_payload\":%zu,\"peak_charged\":%zu,\"current_payload\":%"
           "zu,\"current_charged\":%zu,\"retained_region\":256,\"total_"
           "free\":%zu,\"largest_free\":%zu,\"failures\":%u}\n",
           policy, instrumentation, workload, index,
           (unsigned long long)s->elapsed_ns, s->peak_payload, s->peak_charged,
           s->last.payload, s->last.charged, s->total_free, s->largest_free,
           s->failures);
}

int main(void)
{
    for (unsigned policy = 0; policy < 2; ++policy)
    {
        for (unsigned instrumentation = 0; instrumentation < 2;
             ++instrumentation)
        {
            for (unsigned workload = 0; workload < WORKLOADS; ++workload)
            {
                for (unsigned index = 0; index < SAMPLES; ++index)
                {
                    struct sample s;
                    run_sample(policy, instrumentation, workload, &s);
                    print_sample(policy, instrumentation, workload, index, &s);
                }
            }
        }
    }
    return 0;
}
