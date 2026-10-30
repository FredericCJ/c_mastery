/*
 * Tests for the ring consumer protocol and drain lifecycle (pmu_sampling.h).
 *
 * A producer plays the kernel's role: it reads data_tail with acquire
 * ordering to find free space, writes a complete sample record, then
 * publishes data_head with release ordering. The consumer is the supplied
 * code itself: pf_ring_view_drain directly, and the real drainer thread of
 * pf_sampling_observe with start/stop controls replaced by a model. Build
 * with -fsanitize=thread to check the ordering contract; the assertions
 * check that every record is consumed exactly once, in order, across many
 * wraparounds, that shutdown loses nothing, and that throttle records and
 * failures are attributed correctly. No PMU access is needed.
 */
#define _POSIX_C_SOURCE 200809L
#include "pmu_sampling.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum
{
    TEST_CAPACITY = 512, /* bytes: wraps every 12.8 records */
    TEST_RECORD_SIZE = 40,
    TEST_TYPE_SAMPLE = 9,
    TEST_TYPE_THROTTLE = 5,
    TEST_TYPE_UNTHROTTLE = 6,
    TEST_THROTTLE_SIZE = 32, /* header, time, id, stream_id */
    TEST_RECORDS = 20000,
    TEST_STALL_SECONDS = 5 /* a producer that waits longer has no consumer */
};

static unsigned failure_count;

#define PF_EXPECT(condition)                                              \
    do                                                                    \
    {                                                                     \
        if (!(condition))                                                 \
        {                                                                 \
            (void)fprintf(stderr, "PF_SAMPLING_ASSERT:%d:%s\n", __LINE__, \
                          #condition);                                    \
            failure_count++;                                              \
        }                                                                 \
    } while (0)

struct shared_ring
{
    unsigned char data[TEST_CAPACITY];
    uint64_t head;
    uint64_t tail;
    atomic_bool producer_done;
    atomic_bool consumer_failed; /* the producer stops waiting for space */
    bool stalled; /* producer gave up: no consumer made room in time */
    uint64_t produced;
};

struct consumer_state
{
    uint64_t next_expected_ip;
    uint64_t out_of_order;
};

static void put_header(unsigned char *record, uint32_t type, uint16_t size)
{
    memcpy(record, &type, sizeof type);
    memcpy(record + 6, &size, sizeof size);
}

static void ring_write(unsigned char *data, size_t capacity, uint64_t at,
                       const unsigned char *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        data[(size_t)(at + i) & (capacity - 1u)] = bytes[i];
    }
}

static bool waited_too_long(const struct timespec *start)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
    {
        return true;
    }
    return (now.tv_sec - start->tv_sec) > TEST_STALL_SECONDS;
}

/* Returns false when the consumer has failed or never makes room: with no
 * drain the ring never frees space, so waiting would hang instead of
 * reporting the failure. */
static bool producer_put(struct shared_ring *ring, uint64_t ip)
{
    unsigned char record[TEST_RECORD_SIZE] = {0};
    uint64_t head = __atomic_load_n(&ring->head, __ATOMIC_RELAXED);
    struct timespec start;

    put_header(record, TEST_TYPE_SAMPLE, TEST_RECORD_SIZE);
    memcpy(record + 8, &ip, sizeof ip);
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        return false;
    }
    /* Wait for space; acquire pairs with the consumer's release of tail. */
    while (head + TEST_RECORD_SIZE -
               __atomic_load_n(&ring->tail, __ATOMIC_ACQUIRE) >
           TEST_CAPACITY)
    {
        if (atomic_load_explicit(&ring->consumer_failed,
                                 memory_order_acquire) ||
            waited_too_long(&start))
        {
            ring->stalled = true;
            return false;
        }
        sched_yield();
    }
    ring_write(ring->data, TEST_CAPACITY, head, record, sizeof record);
    __atomic_store_n(&ring->head, head + TEST_RECORD_SIZE, __ATOMIC_RELEASE);
    ring->produced++;
    return true;
}

static void produce(struct shared_ring *ring, uint64_t records)
{
    for (uint64_t ip = 0; ip < records; ++ip)
    {
        if (!producer_put(ring, ip))
        {
            break;
        }
    }
}

static void *producer_main(void *argument)
{
    struct shared_ring *ring = argument;

    produce(ring, TEST_RECORDS);
    atomic_store_explicit(&ring->producer_done, true, memory_order_release);
    return NULL;
}

static void check_order(void *context, const struct pf_sample *sample)
{
    struct consumer_state *state = context;

    if (sample->ip != state->next_expected_ip)
    {
        state->out_of_order++;
    }
    state->next_expected_ip = sample->ip + 1u;
}

static struct pf_ring_view view_of(struct shared_ring *ring)
{
    return (struct pf_ring_view){.data = ring->data,
                                 .capacity = TEST_CAPACITY,
                                 .head = &ring->head,
                                 .tail = &ring->tail};
}

static void init_ring(struct shared_ring *ring)
{
    memset(ring, 0, sizeof *ring);
    atomic_init(&ring->producer_done, false);
    atomic_init(&ring->consumer_failed, false);
}

/* pf_ring_view_drain against a concurrent producer thread. */
static void test_view_drain_concurrent(void)
{
    static struct shared_ring ring;
    struct pf_ring_view view;
    struct pf_ring_stats stats = {0};
    struct consumer_state state = {0};
    pthread_t producer;

    init_ring(&ring);
    view = view_of(&ring);
    if (pthread_create(&producer, NULL, producer_main, &ring) != 0)
    {
        PF_EXPECT(!"pthread_create");
        return;
    }
    while (!atomic_load_explicit(&ring.producer_done, memory_order_acquire))
    {
        if (pf_ring_view_drain(&view, 64, &stats, NULL, check_order, &state) !=
            PF_RING_OK)
        {
            (void)fputs("PF_SAMPLING_ASSERT:concurrent drain failed\n", stderr);
            atomic_store_explicit(&ring.consumer_failed, true,
                                  memory_order_release);
            failure_count++;
            break;
        }
    }
    PF_EXPECT(pthread_join(producer, NULL) == 0);
    /* Final drain after the producer has stopped, as after disable. */
    PF_EXPECT(pf_ring_view_drain(&view, SIZE_MAX, &stats, NULL, check_order,
                                 &state) == PF_RING_OK);
    PF_EXPECT(stats.samples == TEST_RECORDS && state.out_of_order == 0u);
    PF_EXPECT(ring.tail == ring.head && stats.lost == 0u);
}

/* A wrapped ring: sample at 96 (split by the wrap), throttle at 136,
 * unthrottle at 168, ending at 200. */
static uint64_t write_kinds(unsigned char *data, size_t capacity)
{
    unsigned char sample[TEST_RECORD_SIZE] = {0};
    unsigned char throttle[TEST_THROTTLE_SIZE] = {0};
    unsigned char unthrottle[TEST_THROTTLE_SIZE] = {0};

    put_header(sample, TEST_TYPE_SAMPLE, TEST_RECORD_SIZE);
    put_header(throttle, TEST_TYPE_THROTTLE, TEST_THROTTLE_SIZE);
    put_header(unthrottle, TEST_TYPE_UNTHROTTLE, TEST_THROTTLE_SIZE);
    ring_write(data, capacity, 96, sample, sizeof sample);
    ring_write(data, capacity, 136, throttle, sizeof throttle);
    ring_write(data, capacity, 168, unthrottle, sizeof unthrottle);
    return 200;
}

static void test_count_kinds(void)
{
    unsigned char data[128] = {0};
    struct pf_ring_kinds kinds = {0};
    uint64_t end = write_kinds(data, sizeof data);

    pf_ring_count_kinds(data, sizeof data, 96, end, &kinds);
    PF_EXPECT(kinds.throttle == 1u && kinds.unthrottle == 1u);

    /* Only the consumed range counts. */
    kinds = (struct pf_ring_kinds){0};
    pf_ring_count_kinds(data, sizeof data, 96, 168, &kinds);
    PF_EXPECT(kinds.throttle == 1u && kinds.unthrottle == 0u);

    /* A malformed header stops the count; a range wider than the ring is
     * not a consumed range and counts nothing. */
    unsigned char zero_size[8] = {0};
    ring_write(data, sizeof data, 136, zero_size, sizeof zero_size);
    kinds = (struct pf_ring_kinds){0};
    pf_ring_count_kinds(data, sizeof data, 96, end, &kinds);
    PF_EXPECT(kinds.throttle == 0u && kinds.unthrottle == 0u);
    pf_ring_count_kinds(data, sizeof data, 0, 1024, &kinds);
    PF_EXPECT(kinds.throttle == 0u && kinds.unthrottle == 0u);
}

static void test_view_drain_counts_kinds(void)
{
    unsigned char data[128] = {0};
    uint64_t head = write_kinds(data, sizeof data);
    uint64_t tail = 96;
    struct pf_ring_view view = {
        .data = data, .capacity = sizeof data, .head = &head, .tail = &tail};
    struct pf_ring_stats stats = {0};
    struct pf_ring_kinds kinds = {0};

    PF_EXPECT(pf_ring_view_drain(&view, SIZE_MAX, &stats, &kinds, NULL, NULL) ==
              PF_RING_OK);
    PF_EXPECT(tail == head && stats.samples == 1u && stats.unknown == 2u);
    PF_EXPECT(kinds.throttle == 1u && kinds.unthrottle == 1u);
}

/* --- the real drain lifecycle with a model in place of the kernel ------- */

struct model
{
    struct shared_ring ring;
    uint64_t records; /* produced by the workload */
    int start_errno; /* nonzero: start fails with this errno */
    bool ran;
};

static int model_start(void *context)
{
    struct model *model = context;

    if (model->start_errno != 0)
    {
        errno = model->start_errno;
        return -1;
    }
    return 0;
}

static int model_stop(void *context)
{
    (void)context;
    return 0;
}

static void model_workload(void *context)
{
    struct model *model = context;

    model->ran = true;
    produce(&model->ring, model->records);
}

static enum pf_sampling_stage observe_model(struct model *model,
                                            enum pf_drain_mode drain,
                                            struct pf_sampling_result *result)
{
    struct pf_sampling_config config = {
        .drain = drain, .poll_interval_ms = 1, .data_pages = 1};
    struct pf_ring_view view = view_of(&model->ring);
    struct pf_sampling_control control = {
        .context = model, .start = model_start, .stop = model_stop};
    int wake[2];
    enum pf_sampling_stage stage;

    memset(result, 0, sizeof *result);
    if (pipe(wake) != 0)
    {
        PF_EXPECT(!"pipe");
        return PF_SAMPLING_THREAD;
    }
    /* One unread byte keeps the descriptor readable: the drainer never
     * sleeps, which maximizes interleavings for ThreadSanitizer. */
    PF_EXPECT(write(wake[1], "x", 1) == 1);
    stage = pf_sampling_observe(&config, &view, wake[0], &control,
                                model_workload, model, result);
    PF_EXPECT(close(wake[0]) == 0 && close(wake[1]) == 0);
    return stage;
}

static void test_observe_concurrent(void)
{
    static struct model model;
    struct pf_sampling_result result;

    init_ring(&model.ring);
    model.records = TEST_RECORDS;
    model.start_errno = 0;
    model.ran = false;
    PF_EXPECT(observe_model(&model, PF_DRAIN_CONCURRENT, &result) ==
              PF_SAMPLING_COMPLETE);
    PF_EXPECT(!model.ring.stalled && model.ring.produced == TEST_RECORDS);
    PF_EXPECT(result.ring.samples == TEST_RECORDS && result.drains >= 2u);
    PF_EXPECT(result.parser_ran && result.parser_status == PF_RING_OK);
    PF_EXPECT(model.ring.tail == model.ring.head);
}

static void test_observe_final(void)
{
    static struct model model;
    struct pf_sampling_result result;

    init_ring(&model.ring);
    model.records = 12; /* 480 bytes: fits without a consumer */
    model.start_errno = 0;
    PF_EXPECT(observe_model(&model, PF_DRAIN_FINAL, &result) ==
              PF_SAMPLING_COMPLETE);
    PF_EXPECT(result.ring.samples == 12u && result.drains == 1u);
}

static void test_observe_control_failure(void)
{
    static struct model model;
    struct pf_sampling_result result;

    init_ring(&model.ring);
    model.records = 12;
    model.start_errno = EIO;
    model.ran = false;
    PF_EXPECT(observe_model(&model, PF_DRAIN_CONCURRENT, &result) ==
              PF_SAMPLING_CONTROL);
    PF_EXPECT(!model.ran && result.control_errno == EIO);
    /* The ring is still drained, and nothing was there to drain. */
    PF_EXPECT(result.parser_ran && result.ring.samples == 0u);
}

static void never_runs(void *context)
{
    *(int *)context = 1;
}

/* A run that fails before any event is opened has observed nothing: its
 * loss accounting must be unknown, never a complete zero, and the failure
 * belongs to the configuration, not to the parser. No PMU needed: three
 * data pages are rejected before perf_event_open is called. */
static void test_early_failure_reports_unknown_loss(void)
{
    struct pf_sampling_config config = {
        .sample_period_cycles = 100000,
        .data_pages = 3,
        .request_lost_count = true,
    };
    struct pf_sampling_result result;
    int ran = 0;

    memset(&result, 0, sizeof result);
    pf_sampling_run(&config, never_runs, &ran, &result);
    PF_EXPECT(ran == 0 && strcmp(result.status, "invalid") == 0);
    PF_EXPECT(result.loss.accounting == PF_LOSS_UNKNOWN);
    PF_EXPECT(!result.loss.lossless && !result.reading.lost_known);
    PF_EXPECT(result.stage == PF_SAMPLING_CONFIG && !result.parser_ran);
}

int main(void)
{
    test_count_kinds();
    test_view_drain_counts_kinds();
    test_early_failure_reports_unknown_loss();
    test_view_drain_concurrent();
    test_observe_final();
    test_observe_control_failure();
    test_observe_concurrent();
    if (failure_count != 0u)
    {
        (void)fprintf(stderr, "PF_SAMPLING_FAIL:%u\n", failure_count);
        return EXIT_FAILURE;
    }
    (void)puts("PF_SAMPLING_PASS");
    return EXIT_SUCCESS;
}
