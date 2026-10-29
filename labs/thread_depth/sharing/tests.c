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

static void partition(void)
{
    int32_t input[16];
    for (unsigned i = 0; i < 16; i++)
    {
        input[i] = (int32_t)i - 8;
    }
    for (size_t n = 0; n <= 16; n++)
    {
        for (unsigned w = 1; w <= 4; w++)
        {
            for (int mode = 0; mode <= 1; mode++)
            {
                int64_t expected = 0, out = 123;
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
    CHECK(ts_sum(input, 17, 4, 0, &out) == EINVAL && out == 77);
    CHECK(ts_sum(input, 16, 0, 0, &out) == EINVAL && out == 77);
    struct ts_input p = {.values = {1, 2, 3, 4}, .length = 4, .version = 7};
    CHECK(!pthread_mutex_init(&p.mutex, NULL));
    struct tw_mutation m = {.input = &p};
    unsigned version = 0;
    CHECK(!ts_snapshot_sum(&p, 4, mutate_after_copy, &m, &out, &version));
    CHECK(!pthread_join(m.thread, NULL));
    CHECK(out == 10 && version == 7 && p.values[0] == 99 && p.version == 8);
    CHECK(!pthread_mutex_destroy(&p.mutex));
    puts(
        "OBS partition 136 contracts snapshot=10 version=7 original-mutated=8");
}

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
    unsigned id = 99, state = 99, terminal = 99, settled = 99;
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

static void histogram(void)
{
    uint32_t values[255];
    for (unsigned i = 0; i < 255; i++)
    {
        values[i] = (i * i + 3 * i) % 17;
    }
    for (size_t n = 0; n <= 255; n = n == 0    ? 1
                                     : n == 1  ? 16
                                     : n == 16 ? 255
                                               : 256)
    {
        for (unsigned mode = 0; mode < 4; mode++)
        {
            uint64_t expected[17] = {0}, out[17];
            for (size_t i = 0; i < n; i++)
            {
                expected[values[i]]++;
            }
            struct ts_resources resource = {0};
            CHECK(!ts_histogram(n ? values : NULL, n, 17, mode,
                                4u * 1024u * 1024u, out, &resource));
            for (size_t b = 0; b < 17; b++)
            {
                CHECK(out[b] == expected[b]);
            }
            CHECK(resource.requested_buffer_bytes ==
                      n * 4 + (mode == TS_PRIVATE ? 5u : 2u) * 17u * 8 &&
                  resource.workers == 4 &&
                  resource.requested_stack_bytes == 4u * 256u * 1024u);
            for (size_t b = 0; b < 17; b++)
            {
                out[b] = 99;
            }
            struct ts_resources sentinel = {123, 123, 123};
            resource = sentinel;
            CHECK(ts_histogram(n ? values : NULL, n, 17, mode, 1, out,
                               &resource) == ENOMEM);
            CHECK(out[0] == 99 && resource.requested_buffer_bytes == 123);
        }
    }
    uint64_t out[17] = {99};
    struct ts_resources resource = {0};
    values[0] = 17;
    CHECK(ts_histogram(values, 1, 17, TS_SHARED, 4096, out, &resource) ==
              EINVAL &&
          out[0] == 99);
    puts(
        "OBS histogram 16 equivalent contracts all-counts scalar-oracle matched budget-rejection-preserves-output");
}

static uint64_t now(void)
{
    struct timespec t;
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

static void measure(void)
{
    long cache = sysconf(_SC_LEVEL1_DCACHE_SIZE),
         cpus = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(cache > 0);
    printf("ENV L1D=%ld online_cpus=%ld affinity=unmodified\n", cache, cpus);
    size_t n = 262144;
    uint32_t *input = malloc(n * sizeof *input);
    CHECK(input);
    for (unsigned size = 0; size < 2; size++)
    {
        size_t bins = size ? 65536 : 64;
        CHECK(size == 0 || bins * sizeof(uint64_t) > (size_t)cache);
        uint64_t *expected = calloc(bins, sizeof *expected),
                 *out = malloc(bins * sizeof *out);
        CHECK(expected && out);
        for (unsigned distribution = 0; distribution < 2; distribution++)
        {
            memset(expected, 0, bins * sizeof *expected);
            for (size_t i = 0; i < n; i++)
            {
                input[i] = distribution ? (uint32_t)(i % 8)
                                        : (uint32_t)((i * 2654435761u) % bins);
                expected[input[i]]++;
            }
            for (unsigned warm = 0; warm < 4; warm++)
            {
                struct ts_resources r;
                CHECK(!ts_histogram(input, n, bins, warm, 4u * 1024u * 1024u,
                                    out, &r));
            }
            for (unsigned trial = 0; trial < 3; trial++)
            {
                for (unsigned order = 0; order < 4; order++)
                {
                    unsigned mode = (trial + order) % 4;
                    struct ts_resources r;
                    uint64_t begin = now();
                    CHECK(!ts_histogram(input, n, bins, mode,
                                        4u * 1024u * 1024u, out, &r));
                    uint64_t end = now();
                    for (size_t b = 0; b < bins; b++)
                    {
                        CHECK(out[b] == expected[b]);
                    }
                    printf(
                        "SAMPLE bins=%zu distribution=%u mode=%u trial=%u ns=%" PRIu64
                        " buffers=%zu requested_stacks=%zu\n",
                        bins, distribution, mode, trial, end - begin,
                        r.requested_buffer_bytes, r.requested_stack_bytes);
                }
            }
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
