#define _GNU_SOURCE
#include "sharing.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TS_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

enum
{
    SUM_MAX_ELEMENTS = 16,
    MAX_WORKERS = 4,
    HISTOGRAM_BINS = 17,
    HISTOGRAM_BUDGET_BYTES = 4 * 1024 * 1024,
    STACK_REQUEST_BYTES = 256 * 1024,
    MEASURE_ITEMS = 262144,
    NSEC_PER_SEC = 1000000000
};

/* Writer used from the snapshot hook: changes every value and the version
 * after ts_snapshot_sum has already copied its input. */
static void *mutate(void *arg)
{
    struct ts_input *p = arg;
    CHECK(!pthread_mutex_lock(&p->mutex));
    for (size_t i = 0; i < p->length; i++)
    {
        p->values[i] = 99;
    }
    p->version = 8;
    CHECK(!pthread_mutex_unlock(&p->mutex));
    return NULL;
}

/* Carries the writer thread started by the snapshot hook back to the test
 * so it can be joined. */
struct tw_mutation
{
    struct ts_input *input;
    pthread_t thread;
};

static void mutate_after_copy(void *arg)
{
    struct tw_mutation *m = arg;
    CHECK(!pthread_create(&m->thread, NULL, mutate, m->input));
}

/* Every length 0..16, every worker count and both accumulation modes must
 * match the scalar sum; invalid shapes are rejected without touching *out. */
static void partition_sums(void)
{
    int32_t input[SUM_MAX_ELEMENTS];
    for (unsigned i = 0; i < SUM_MAX_ELEMENTS; i++)
    {
        input[i] = (int32_t)i - 8;
    }
    for (size_t n = 0; n <= SUM_MAX_ELEMENTS; n++)
    {
        for (unsigned w = 1; w <= MAX_WORKERS; w++)
        {
            for (int mode = 0; mode <= 1; mode++)
            {
                int64_t expected = 0;
                int64_t out = 123;
                for (size_t i = 0; i < n; i++)
                {
                    expected += input[i];
                }
                CHECK(!ts_sum(n ? input : NULL, n, w, mode, &out));
                CHECK(out == expected);
            }
        }
    }
    int64_t out = 77;
    CHECK(ts_sum(input, SUM_MAX_ELEMENTS + 1, 4, 0, &out) == EINVAL &&
          out == 77);
    CHECK(ts_sum(input, SUM_MAX_ELEMENTS, 0, 0, &out) == EINVAL && out == 77);
}

/* The hook starts a writer after the snapshot's copy: the sum and version
 * must describe the copy, while the original shows the writer's changes. */
static void snapshot_isolation(void)
{
    int64_t out = 77;
    struct ts_input p = {.values = {1, 2, 3, 4}, .length = 4, .version = 7};
    CHECK(!pthread_mutex_init(&p.mutex, NULL));
    struct tw_mutation m = {.input = &p};
    unsigned version = 0;
    CHECK(!ts_snapshot_sum(&p, 4, mutate_after_copy, &m, &out, &version));
    CHECK(!pthread_join(m.thread, NULL));
    CHECK(out == 10 && version == 7 && p.values[0] == 99 && p.version == 8);
    CHECK(!pthread_mutex_destroy(&p.mutex));
}

static void partition(void)
{
    partition_sums();
    snapshot_isolation();
    puts(
        "OBS partition 136 contracts snapshot=10 version=7 original-mutated=8");
}

/* Worker that takes id 0, parks between two barrier crossings (so the main
 * thread can close the queue while the id is in flight), then settles it. */
struct tw_worker
{
    struct ts_queue *queue;
    pthread_barrier_t *stage;
};

static void barrier(pthread_barrier_t *p)
{
    int e = pthread_barrier_wait(p);
    CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
}

static void *fail_after_take(void *arg)
{
    struct tw_worker *w = arg;
    unsigned id = 99;
    CHECK(!ts_dequeue(w->queue, &id) && id == 0);
    barrier(w->stage);
    barrier(w->stage);
    CHECK(!ts_settle(w->queue, id, TS_FAILED));
    return NULL;
}

static void ownership(void)
{
    struct ts_queue q;
    CHECK(!ts_queue_init(&q));
    unsigned id = 99;
    unsigned state = 99;
    unsigned terminal = 99;
    unsigned settled = 99;
    CHECK(ts_dequeue(&q, &id) == EAGAIN && id == 99);
    CHECK(!ts_enqueue(&q, 0));
    CHECK(!ts_enqueue(&q, 1));
    CHECK(ts_enqueue(&q, 2) == EAGAIN);
    CHECK(!ts_queue_inspect(&q, 2, &state, &terminal, &settled));
    CHECK(state == TS_CALLER && terminal == 0 && settled == 0);
    CHECK(ts_enqueue(&q, 0) == EALREADY);
    CHECK(ts_queue_destroy(&q) == EBUSY);
    pthread_barrier_t stage;
    CHECK(!pthread_barrier_init(&stage, NULL, 2));
    struct tw_worker w = {&q, &stage};
    pthread_t worker;
    CHECK(!pthread_create(&worker, NULL, fail_after_take, &w));
    barrier(&stage);
    CHECK(!ts_queue_close(&q));
    CHECK(ts_enqueue(&q, 2) == EPIPE);
    CHECK(ts_dequeue(&q, &id) == EPIPE);
    CHECK(!ts_queue_inspect(&q, 0, &state, &terminal, &settled) &&
          state == TS_WORKER && settled == 0);
    CHECK(!ts_queue_inspect(&q, 1, &state, &terminal, &settled) &&
          state == TS_SETTLED && terminal == TS_CANCELED && settled == 1);
    barrier(&stage);
    CHECK(!pthread_join(worker, NULL));
    CHECK(!pthread_barrier_destroy(&stage));
    CHECK(!ts_queue_inspect(&q, 0, &state, &terminal, &settled) &&
          state == TS_SETTLED && terminal == TS_FAILED && settled == 1);
    CHECK(ts_settle(&q, 0, TS_SUCCESS) == EALREADY);
    CHECK(!ts_queue_close(&q));
    CHECK(!ts_queue_destroy(&q));
    puts(
        "OBS ownership rejected=caller queued-close=canceled worker-failure=failed terminal-per-accepted-id=1");
}

/* Lengths 0, 1, 16 and 255 (a boundary sequence, not a range). */
static size_t next_histogram_length(size_t n)
{
    if (n == 0)
    {
        return 1;
    }
    if (n == 1)
    {
        return 16;
    }
    if (n == 16)
    {
        return 255;
    }
    return 256;
}

/* One (length, mode) case: equals the scalar histogram, reports the exact
 * requested resources, and a too-small budget changes nothing. */
static void histogram_case(const uint32_t *values, size_t n, unsigned mode)
{
    uint64_t expected[HISTOGRAM_BINS] = {0};
    uint64_t out[HISTOGRAM_BINS];
    for (size_t i = 0; i < n; i++)
    {
        expected[values[i]]++;
    }
    struct ts_resources resource = {0};
    CHECK(!ts_histogram(n ? values : NULL, n, HISTOGRAM_BINS, mode,
                        HISTOGRAM_BUDGET_BYTES, out, &resource));
    for (size_t b = 0; b < HISTOGRAM_BINS; b++)
    {
        CHECK(out[b] == expected[b]);
    }
    size_t bin_arrays = mode == TS_PRIVATE ? 5u : 2u;
    CHECK(resource.requested_buffer_bytes ==
              n * sizeof values[0] +
                  bin_arrays * HISTOGRAM_BINS * sizeof(uint64_t) &&
          resource.workers == MAX_WORKERS &&
          resource.requested_stack_bytes == MAX_WORKERS * STACK_REQUEST_BYTES);
    for (size_t b = 0; b < HISTOGRAM_BINS; b++)
    {
        out[b] = 99;
    }
    struct ts_resources sentinel = {123, 123, 123};
    resource = sentinel;
    CHECK(ts_histogram(n ? values : NULL, n, HISTOGRAM_BINS, mode, 1, out,
                       &resource) == ENOMEM);
    CHECK(out[0] == 99 && resource.requested_buffer_bytes == 123);
}

static void histogram(void)
{
    uint32_t values[255];
    for (unsigned i = 0; i < 255; i++)
    {
        values[i] = (i * i + 3 * i) % HISTOGRAM_BINS;
    }
    for (size_t n = 0; n <= 255; n = next_histogram_length(n))
    {
        for (unsigned mode = 0; mode < 4; mode++)
        {
            histogram_case(values, n, mode);
        }
    }
    uint64_t out[HISTOGRAM_BINS] = {99};
    struct ts_resources resource = {0};
    values[0] = HISTOGRAM_BINS;
    CHECK(ts_histogram(values, 1, HISTOGRAM_BINS, TS_SHARED, 4096, out,
                       &resource) == EINVAL &&
          out[0] == 99);
    puts(
        "OBS histogram 16 equivalent contracts all-counts scalar-oracle matched budget-rejection-preserves-output");
}

static uint64_t now(void)
{
    struct timespec t;
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
    return (uint64_t)t.tv_sec * NSEC_PER_SEC + (uint64_t)t.tv_nsec;
}

/* Deterministic key streams: distribution 0 spreads keys over all bins,
 * distribution 1 concentrates them on 8 hot keys. Also fills the scalar
 * oracle `expected`. */
static void fill_input(uint32_t *input, uint64_t *expected, size_t bins,
                       unsigned distribution)
{
    memset(expected, 0, bins * sizeof *expected);
    for (size_t i = 0; i < MEASURE_ITEMS; i++)
    {
        if (distribution)
        {
            input[i] = (uint32_t)(i % 8);
        }
        else
        {
            input[i] = (uint32_t)((i * 2654435761u) % bins);
        }
        expected[input[i]]++;
    }
}

/* Warm every mode once, then time 3 trials of all 4 modes with the mode order
 * rotated per trial so no mode always runs first. Prints one SAMPLE line per
 * timed run; elapsed time is in nanoseconds and the caller never ranks them. */
static void time_modes(const uint32_t *input, const uint64_t *expected,
                       uint64_t *out, size_t bins, unsigned distribution)
{
    for (unsigned warm = 0; warm < 4; warm++)
    {
        struct ts_resources r;
        CHECK(!ts_histogram(input, MEASURE_ITEMS, bins, warm,
                            HISTOGRAM_BUDGET_BYTES, out, &r));
    }
    for (unsigned trial = 0; trial < 3; trial++)
    {
        for (unsigned order = 0; order < 4; order++)
        {
            unsigned mode = (trial + order) % 4;
            struct ts_resources r;
            uint64_t begin_ns = now();
            CHECK(!ts_histogram(input, MEASURE_ITEMS, bins, mode,
                                HISTOGRAM_BUDGET_BYTES, out, &r));
            uint64_t end_ns = now();
            for (size_t b = 0; b < bins; b++)
            {
                CHECK(out[b] == expected[b]);
            }
            printf(
                "SAMPLE bins=%zu distribution=%u mode=%u trial=%u ns=%" PRIu64
                " buffers=%zu requested_stacks=%zu\n",
                bins, distribution, mode, trial, end_ns - begin_ns,
                r.requested_buffer_bytes, r.requested_stack_bytes);
        }
    }
}

static void measure(void)
{
    long cache = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(cache > 0);
    printf("ENV L1D=%ld online_cpus=%ld affinity=unmodified\n", cache, cpus);
    uint32_t *input = malloc(MEASURE_ITEMS * sizeof *input);
    CHECK(input);
    for (unsigned size = 0; size < 2; size++)
    {
        /* Large case: the bin array alone exceeds the L1 data cache. */
        size_t bins = size ? 65536 : 64;
        CHECK(size == 0 || bins * sizeof(uint64_t) > (size_t)cache);
        uint64_t *expected = calloc(bins, sizeof *expected);
        uint64_t *out = malloc(bins * sizeof *out);
        CHECK(expected && out);
        for (unsigned distribution = 0; distribution < 2; distribution++)
        {
            fill_input(input, expected, bins, distribution);
            time_modes(input, expected, out, bins, distribution);
        }
        free(out);
        free(expected);
    }
    free(input);
    puts("OBS measurement 48 raw samples no-universal-speedup-claim");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    int previous;
    CHECK(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous));
    if (!strcmp(argv[1], "partition"))
    {
        partition();
    }
    else if (!strcmp(argv[1], "ownership"))
    {
        ownership();
    }
    else if (!strcmp(argv[1], "histogram"))
    {
        histogram();
    }
    else if (!strcmp(argv[1], "measurement"))
    {
        measure();
    }
    else
    {
        CHECK(0);
    }
    CHECK(!pthread_setcancelstate(previous, NULL));
    return 0;
}
