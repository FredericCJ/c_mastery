/*
 * Concurrency test for the ring consumer protocol (pmu_sampling.h).
 *
 * A producer thread plays the kernel's role: it reads data_tail with acquire
 * ordering to find free space, writes a complete sample record, then
 * publishes data_head with release ordering. A consumer thread drains through
 * pf_ring_view_drain exactly as the collector's drainer does. Build with
 * -fsanitize=thread to check the ordering contract; the assertions check that
 * every record is consumed exactly once, in order, across many wraparounds,
 * and that shutdown loses nothing.
 */
#include "pmu_sampling.h"

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    TEST_CAPACITY = 512, /* bytes: wraps every 12.8 records */
    TEST_RECORD_SIZE = 40,
    TEST_TYPE_SAMPLE = 9,
    TEST_RECORDS = 20000
};

struct shared_ring
{
    unsigned char data[TEST_CAPACITY];
    uint64_t head;
    uint64_t tail;
    atomic_bool producer_done;
    atomic_bool consumer_failed; /* the producer stops waiting for space */
};

struct consumer_state
{
    uint64_t next_expected_ip;
    uint64_t out_of_order;
};

/* Returns false when the consumer has failed: with no drain the ring never
 * frees space, so waiting would hang instead of reporting the failure. */
static bool producer_put(struct shared_ring *ring, uint64_t ip)
{
    unsigned char record[TEST_RECORD_SIZE] = {0};
    uint32_t type = TEST_TYPE_SAMPLE;
    uint16_t size = TEST_RECORD_SIZE;
    uint64_t head = __atomic_load_n(&ring->head, __ATOMIC_RELAXED);

    memcpy(record, &type, sizeof type);
    memcpy(record + 6, &size, sizeof size);
    memcpy(record + 8, &ip, sizeof ip);
    /* Wait for space; acquire pairs with the consumer's release of tail. */
    while (head + TEST_RECORD_SIZE -
               __atomic_load_n(&ring->tail, __ATOMIC_ACQUIRE) >
           TEST_CAPACITY)
    {
        if (atomic_load_explicit(&ring->consumer_failed, memory_order_acquire))
        {
            return false;
        }
        sched_yield();
    }
    for (size_t i = 0; i < sizeof record; ++i)
    {
        ring->data[(size_t)(head + i) & (TEST_CAPACITY - 1u)] = record[i];
    }
    __atomic_store_n(&ring->head, head + TEST_RECORD_SIZE, __ATOMIC_RELEASE);
    return true;
}

static void *producer_main(void *argument)
{
    struct shared_ring *ring = argument;

    for (uint64_t ip = 0; ip < TEST_RECORDS; ++ip)
    {
        if (!producer_put(ring, ip))
        {
            break;
        }
    }
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

static void never_runs(void *context)
{
    *(int *)context = 1;
}

/* A run that fails before any event is opened has observed nothing: its
 * loss accounting must be unknown, never a complete zero. No PMU needed:
 * three data pages are rejected before perf_event_open is called. */
static int test_early_failure_reports_unknown_loss(void)
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
    if ((ran != 0) || (strcmp(result.status, "invalid") != 0) ||
        (result.loss.accounting != PF_LOSS_UNKNOWN) || result.loss.lossless ||
        result.reading.lost_known)
    {
        (void)fprintf(stderr,
                      "PF_SAMPLING_ASSERT:early failure ran=%d status=%s "
                      "accounting=%s lossless=%d\n",
                      ran, result.status,
                      pf_loss_accounting_name(result.loss.accounting),
                      (int)result.loss.lossless);
        return 1;
    }
    return 0;
}

int main(void)
{
    static struct shared_ring ring;
    struct pf_ring_view view = {
        .data = ring.data,
        .capacity = TEST_CAPACITY,
        .head = &ring.head,
        .tail = &ring.tail,
    };
    struct pf_ring_stats stats = {0};
    struct consumer_state state = {0};
    pthread_t producer;
    int failures = 0;

    failures += test_early_failure_reports_unknown_loss();
    atomic_init(&ring.producer_done, false);
    atomic_init(&ring.consumer_failed, false);
    if (pthread_create(&producer, NULL, producer_main, &ring) != 0)
    {
        (void)fputs("PF_SAMPLING_ASSERT:pthread_create\n", stderr);
        return EXIT_FAILURE;
    }
    /* Concurrent phase: drain while the producer runs. */
    while (!atomic_load_explicit(&ring.producer_done, memory_order_acquire))
    {
        if (pf_ring_view_drain(&view, 64, &stats, check_order, &state) !=
            PF_RING_OK)
        {
            (void)fputs("PF_SAMPLING_ASSERT:concurrent drain failed\n", stderr);
            atomic_store_explicit(&ring.consumer_failed, true,
                                  memory_order_release);
            failures++;
            break;
        }
    }
    if (pthread_join(producer, NULL) != 0)
    {
        failures++;
    }
    /* Final drain after the producer has stopped, as after disable. */
    if (pf_ring_view_drain(&view, SIZE_MAX, &stats, check_order, &state) !=
        PF_RING_OK)
    {
        failures++;
    }
    if ((stats.samples != TEST_RECORDS) || (state.out_of_order != 0u) ||
        (ring.tail != ring.head) || (stats.lost != 0u))
    {
        (void)fprintf(stderr,
                      "PF_SAMPLING_ASSERT:samples=%llu out_of_order=%llu "
                      "tail=%llu head=%llu\n",
                      (unsigned long long)stats.samples,
                      (unsigned long long)state.out_of_order,
                      (unsigned long long)ring.tail,
                      (unsigned long long)ring.head);
        failures++;
    }
    if (failures != 0)
    {
        return EXIT_FAILURE;
    }
    (void)puts("PF_SAMPLING_PASS");
    return EXIT_SUCCESS;
}
