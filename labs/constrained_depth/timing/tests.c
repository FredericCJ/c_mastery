#include "timing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("CT_CHECK:%u:%s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

struct cr_hook
{
    struct ct_clock *clock;
    unsigned calls;
};

static void change(void *context, unsigned phase)
{
    struct cr_hook *h = context;
    if (phase == 0 && !h->calls++)
    {
        struct ct_stamp s = {7, 1, 1000, 0, 8};
        if (ct_publish(h->clock, s) != CT_OK)
        {
            exit(2);
        }
    }
}

static int same(struct ct_stamp a, struct ct_stamp b)
{
    return a.boot == b.boot && a.epoch == b.epoch &&
           a.quantum_ns == b.quantum_ns && a.low == b.low && a.bits == b.bits;
}

int main(void)
{
    uint32_t ticks = 99;
    unsigned order = 99;
    CHECK(ct_timeout(1500000, 8, 1000000, &ticks) == CT_OK && ticks == 2);
    CHECK(ct_timeout(127000001, 8, 1000000, &ticks) == CT_RANGE && ticks == 2);
    CHECK(ct_timeout(1500000, 16, 250000, &ticks) == CT_OK && ticks == 6);
    CHECK(ct_timeout(UINT64_MAX, 16, 1, &ticks) == CT_RANGE && ticks == 6);
    CHECK(ct_order(8, 4, 250, &order) == CT_OK && order == CT_DUE);
    CHECK(ct_order(8, 250, 4, &order) == CT_OK && order == CT_FUTURE);
    CHECK(ct_order(8, 250, 250, &order) == CT_OK && order == CT_EQUAL);
    order = 99;
    CHECK(ct_order(8, 128, 0, &order) == CT_AMBIGUOUS && order == 99);
    CHECK(ct_order(8, 256, 0, &order) == CT_INVALID && order == 99);
    CHECK(ct_elapsed(8, 250, 4, 1, &ticks) == CT_OK && ticks == 10);
    CHECK(ct_elapsed(8, 250, 4, 0, &ticks) == CT_AMBIGUOUS && ticks == 10);
    CHECK((250u + 10u) % 256 == (250u + 266u) % 256);
    struct ct_clock clock;
    struct ct_stamp first = {7, 0, 1000, 255, 8}, out = {99, 99, 99, 99, 99};
    CHECK(ct_clock_init(&clock, first) == CT_OK);
    struct cr_hook hook = {&clock, 0};
    CHECK(ct_snapshot(&clock, 1, change, &hook, &out) == CT_BUSY &&
          out.boot == 99);
    CHECK(ct_snapshot(&clock, 2, NULL, NULL, &out) == CT_OK && out.epoch == 1 &&
          out.low == 0);
    CHECK(ct_clock_init(&clock, first) == CT_OK);
    hook.calls = 0;
    CHECK(ct_snapshot(&clock, 2, change, &hook, &out) == CT_OK &&
          out.epoch == 1 && out.low == 0);
    /* Deliberately unvalidated split read is defined (both fields atomic), torn. */
    CHECK(ct_clock_init(&clock, first) == CT_OK);
    uint64_t high = atomic_load(&clock.epoch);
    hook.calls = 0;
    change(&hook, 0);
    uint32_t low = atomic_load(&clock.low);
    CHECK(high * 256 + low == 0);
    printf("CT_TORN:unvalidated=0 consistent=256\n");
    struct ct_stamp far = {7, 3, 1000, 4, 8};
    uint64_t duration = 99;
    CHECK(ct_duration(first, far, &duration) == CT_OK && duration == 517000);
    struct ct_stamp reset = far;
    reset.boot = 8;
    CHECK(ct_duration(first, reset, &duration) == CT_DIFFERENT &&
          duration == 517000);
    CHECK(ct_publish(&clock, reset) == CT_OK);
    CHECK(ct_publish(&clock, far) == CT_DIFFERENT);
    CHECK(ct_snapshot(&clock, 1, NULL, NULL, &out) == CT_OK &&
          same(out, reset));
    atomic_store(&clock.sequence, UINT64_MAX - 3);
    reset.low++;
    CHECK(ct_publish(&clock, reset) == CT_OK &&
          atomic_load(&clock.sequence) == UINT64_MAX - 1);
    reset.low++;
    CHECK(ct_publish(&clock, reset) == CT_RANGE);
    CHECK(ct_snapshot(&clock, 1, NULL, NULL, &out) == CT_OK && out.low == 5);
    struct ct_stamp too_far = {1, UINT64_MAX, 1, 0, 16};
    CHECK(ct_duration(too_far, too_far, &duration) == CT_RANGE);
    struct ct_stamp too_long = {1, 1, UINT64_MAX, 0, 8}, zero = too_long;
    zero.epoch = 0;
    CHECK(ct_duration(zero, too_long, &duration) == CT_RANGE);
    unsigned char bytes[40], saved[40];
    CHECK(ct_encode(far, bytes) == CT_OK &&
          ct_decode(bytes, 40, &out) == CT_OK && same(out, far));
    memcpy(saved, bytes, 40);
    struct ct_stamp before = out;
    bytes[28] = 7;
    CHECK(ct_decode(bytes, 40, &out) == CT_INVALID && same(out, before));
    CHECK(ct_decode(saved, 39, &out) == CT_INVALID && same(out, before));
    struct ct_metrics metrics = {0};
    struct ct_slice base[] = {{12, 15, CT_CPU}};
    CHECK(ct_metrics(10, 12, 15, base, 1, &metrics) == CT_OK &&
          metrics.latency == 2 && metrics.elapsed == 3 &&
          metrics.response == 5 && metrics.cpu == 3);
    struct ct_slice interrupt[] = {{12, 13, CT_CPU},
                                   {13, 14, CT_ISR},
                                   {14, 15, CT_CPU},
                                   {15, 16, CT_ISR},
                                   {16, 17, CT_CPU}};
    CHECK(ct_metrics(10, 12, 17, interrupt, 5, &metrics) == CT_OK &&
          metrics.response == 7 && metrics.elapsed == 5 && metrics.cpu == 3 &&
          metrics.isr == 2);
    struct ct_slice preempt[] = {
        {12, 13, CT_CPU}, {13, 15, CT_PREEMPTED}, {15, 17, CT_CPU}};
    CHECK(ct_metrics(10, 12, 17, preempt, 3, &metrics) == CT_OK &&
          metrics.response == 7 && metrics.cpu == 3 && metrics.preempted == 2);
    struct ct_slice dma[] = {{12, 13, CT_CPU}, {13, 14, CT_ISR},
                             {14, 15, CT_DMA}, {15, 16, CT_CPU},
                             {16, 17, CT_ISR}, {17, 18, CT_CPU}};
    CHECK(ct_metrics(10, 12, 18, dma, 6, &metrics) == CT_OK &&
          metrics.response == 8 && metrics.cpu == 3 && metrics.dma == 1 &&
          metrics.isr == 2);
    struct ct_metrics unchanged = metrics;
    dma[2].begin = 13;
    CHECK(ct_metrics(10, 12, 18, dma, 6, &metrics) == CT_INVALID &&
          !memcmp(&metrics, &unchanged, sizeof metrics));
    CHECK(ct_metrics(10, 10, 10, NULL, 0, &metrics) == CT_OK &&
          metrics.response == 0);
    CHECK(ct_metrics(10, 9, 10, NULL, 0, &metrics) == CT_INVALID);
    struct ct_offset jitter;
    CHECK(ct_jitter(0, 0, &jitter) == CT_OK && !jitter.negative &&
          !jitter.magnitude);
    CHECK(ct_jitter(4, 5, &jitter) == CT_OK && !jitter.negative &&
          jitter.magnitude == 1);
    CHECK(ct_jitter(8, 9, &jitter) == CT_OK && jitter.magnitude == 1);
    CHECK(ct_jitter(UINT64_MAX, 0, &jitter) == CT_OK && jitter.negative &&
          jitter.magnitude == UINT64_MAX);
    uint64_t sample[] = {2, 2, 3, 9}, interleaved[] = {2, 9, 2, 9, 2, 9, 2, 2},
             burst[] = {2, 2, 2, 2, 2, 9, 9, 9};
    struct ct_stats stats, a, b;
    CHECK(ct_stats(sample, 4, 5, &stats) == CT_OK && stats.minimum == 2 &&
          stats.maximum == 9 && stats.mean_integer == 4 &&
          !stats.mean_remainder);
    CHECK(ct_stats(interleaved, 8, 5, &a) == CT_OK &&
          ct_stats(burst, 8, 5, &b) == CT_OK && a.sum == 37 && b.sum == 37 &&
          a.longest_exceedance_run == 1 && b.longest_exceedance_run == 3 &&
          a.exceedance_runs == 3 && b.exceedance_runs == 1);
    uint64_t overflow[] = {UINT64_MAX, 1};
    struct ct_stats prior = stats;
    CHECK(ct_stats(overflow, 2, 0, &stats) == CT_RANGE &&
          !memcmp(&stats, &prior, sizeof stats));
    CHECK(ct_stats(sample, 0, 5, &stats) == CT_INVALID);
    puts(
        "CT_METRICS:latency=2 CPU=3 interrupt=2 wall=5 response=7 DMA-response=8 jitter=0,1,1");
    puts(
        "CT_STATS:n=4 resolution=declared1 min=2 mean=4 max=9 not-WCET burst-runs=3,1 longest=1,3");
    puts("CT_PASS:rollover-epochs-acquisition-and-latency");
    return 0;
}
