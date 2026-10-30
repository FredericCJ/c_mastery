/*
 * Deterministic tests for sampling loss accounting (pmu_loss.h), including a
 * model of the kernel's ring producer that shows why a drain performed only
 * after the event is disabled cannot see its final losses.
 *
 * The producer model follows the documented behaviour (perf_event_open(2)):
 * a record that does not fit is lost and counted; a PERF_RECORD_LOST record
 * carrying the accumulated count is written in front of the next record that
 * fits. It is a teaching model of that contract, not the kernel's code.
 */
#include "pmu.h"
#include "pmu_loss.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned failure_count;

#define PF_EXPECT(condition)                                          \
    do                                                                \
    {                                                                 \
        if (!(condition))                                             \
        {                                                             \
            (void)fprintf(stderr, "PF_LOSS_ASSERT:%d:%s\n", __LINE__, \
                          #condition);                                \
            failure_count++;                                          \
        }                                                             \
    } while (0)

/* --- read() record decoding -------------------------------------------- */

static void test_decode(void)
{
    const uint64_t with_lost[] = {500, 2000, 1500, 7};
    const uint64_t without_lost[] = {500, 2000, 1500};
    const uint64_t bad_time[] = {500, 1000, 1001};
    struct pf_sample_reading reading;

    PF_EXPECT(pf_sample_reading_decode(with_lost, sizeof with_lost, true,
                                       &reading) == PF_READING_OK);
    PF_EXPECT(reading.value == 500 && reading.enabled_ns == 2000);
    PF_EXPECT(reading.running_ns == 1500 && reading.lost_known);
    PF_EXPECT(reading.lost == 7);

    PF_EXPECT(pf_sample_reading_decode(without_lost, sizeof without_lost, false,
                                       &reading) == PF_READING_OK);
    PF_EXPECT(!reading.lost_known && reading.lost == 0);

    /* A record shorter or longer than the accepted format is malformed
     * input, not a reading with a zero loss field. */
    PF_EXPECT(pf_sample_reading_decode(without_lost, sizeof without_lost, true,
                                       &reading) == PF_READING_SHORT);
    PF_EXPECT(pf_sample_reading_decode(with_lost, sizeof with_lost, false,
                                       &reading) == PF_READING_SHORT);
    PF_EXPECT(pf_sample_reading_decode(with_lost, 0, true, &reading) ==
              PF_READING_SHORT);
    PF_EXPECT(pf_sample_reading_decode(bad_time, sizeof bad_time, false,
                                       &reading) == PF_READING_MALFORMED);
}

/* --- reconciliation table ---------------------------------------------- */

static struct pf_loss_account reconcile(uint64_t inband, bool known,
                                        uint64_t kernel, uint64_t hardware)
{
    struct pf_loss_inputs inputs = {
        .ring_parsed = true,
        .inband_lost = inband,
        .inband_lost_records = inband ? 1u : 0u,
        .hardware_lost = hardware,
        .kernel_count_known = known,
        .kernel_lost = kernel,
    };
    struct pf_loss_account account;

    pf_loss_reconcile(&inputs, &account);
    return account;
}

/* Loss is known only for a ring parsed to its end: a parser that stopped
 * early saw only a prefix of the in-band records. */
static void test_unparsed_ring_is_unknown(void)
{
    struct pf_loss_inputs inputs = {
        .ring_parsed = false,
        .kernel_count_known = true,
        .kernel_lost = 0,
    };
    struct pf_loss_account account;

    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN);
    PF_EXPECT(!account.lossless);
    inputs.inband_lost = 9;
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN &&
              account.lower_bound == 9);
    /* A known kernel count still bounds the loss from below. */
    inputs.kernel_lost = 598;
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN);
    PF_EXPECT(account.lower_bound == 598 && !account.lossless);
}

/* Throttled intervals take no samples and are absent from every loss count,
 * so a throttled observation is never lossless; nor is one with records the
 * parser could not interpret. */
static void test_throttling_and_unknown_records_block_lossless(void)
{
    struct pf_loss_inputs inputs = {
        .ring_parsed = true,
        .kernel_count_known = true,
        .kernel_lost = 0,
        .throttle_records = 36,
    };
    struct pf_loss_account account;

    pf_loss_reconcile(&inputs, &account);
    /* The ring loss count is complete; the skipped samples are not. */
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE_EXCEPT_THROTTLED);
    PF_EXPECT(account.throttled && !account.lossless);
    PF_EXPECT(account.lower_bound == 0);
    inputs.kernel_lost = 7;
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE_EXCEPT_THROTTLED);
    PF_EXPECT(account.lower_bound == 7 && !account.lossless);
    inputs.kernel_lost = 0;
    inputs.throttle_records = 0;
    inputs.unknown_records = 1;
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(!account.throttled && !account.lossless);
    inputs.unknown_records = 0;
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.lossless);
}

static void test_reconcile(void)
{
    struct pf_loss_account account;

    /* No kernel count: never report zero or lossless, whatever in-band says. */
    account = reconcile(0, false, 0, 0);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN);
    PF_EXPECT(!account.lossless && account.lower_bound == 0);
    account = reconcile(12, false, 0, 0);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN &&
              account.lower_bound == 12);

    /* Kernel count is authoritative; in-band records are not added to it. */
    account = reconcile(12, true, 40, 0);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE);
    PF_EXPECT(account.total_lost == 40 && account.unreported_lost == 28);
    PF_EXPECT(!account.lossless && account.lower_bound == 40);

    account = reconcile(0, true, 0, 0);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE && account.lossless);

    /* Hardware drops are separate and also prevent a lossless claim. */
    account = reconcile(0, true, 0, 3);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE && !account.lossless);
    PF_EXPECT(account.total_lost == 0 && account.hardware_lost == 3);

    /* More in-band loss than the kernel counted contradicts the
     * single-event ring assumption: report it, do not pick a number. */
    account = reconcile(50, true, 40, 0);
    PF_EXPECT(account.accounting == PF_LOSS_INCONSISTENT && !account.lossless);
    PF_EXPECT(account.lower_bound == 50);
}

/* --- kernel ring producer model ---------------------------------------- */

enum
{
    MODEL_CAPACITY = 256, /* bytes; a power of two like the real data area */
    MODEL_SAMPLE_SIZE = 40,
    MODEL_LOST_SIZE = 24,
    MODEL_TYPE_LOST = 2,
    MODEL_TYPE_SAMPLE = 9
};

struct ring_model
{
    unsigned char data[MODEL_CAPACITY];
    uint64_t head; /* producer position (kernel data_head) */
    uint64_t tail; /* consumer position (user data_tail) */
    uint64_t pending; /* losses not yet reported in-band (kernel rb->lost) */
    uint64_t counted; /* every loss (kernel lost-sample count) */
};

static void model_put(struct ring_model *ring, const void *record, size_t size)
{
    const unsigned char *bytes = record;

    for (size_t i = 0; i < size; ++i)
    {
        ring->data[(size_t)(ring->head + i) & (MODEL_CAPACITY - 1)] = bytes[i];
    }
    ring->head += size;
}

static void model_header(unsigned char *out, uint32_t type, uint16_t size)
{
    uint16_t misc = 0;

    memcpy(out, &type, sizeof type);
    memcpy(out + 4, &misc, sizeof misc);
    memcpy(out + 6, &size, sizeof size);
}

/* One sample attempt, following the documented producer contract. */
static void model_sample(struct ring_model *ring, uint64_t ip)
{
    size_t needed = MODEL_SAMPLE_SIZE + (ring->pending ? MODEL_LOST_SIZE : 0u);
    uint64_t free_bytes = MODEL_CAPACITY - (ring->head - ring->tail);

    if (free_bytes < needed)
    {
        ring->pending++;
        ring->counted++;
        return;
    }
    if (ring->pending != 0)
    {
        unsigned char lost[MODEL_LOST_SIZE] = {0};
        uint64_t id = 1;

        model_header(lost, MODEL_TYPE_LOST, MODEL_LOST_SIZE);
        memcpy(lost + 8, &id, sizeof id);
        memcpy(lost + 16, &ring->pending, sizeof ring->pending);
        model_put(ring, lost, sizeof lost);
        ring->pending = 0;
    }
    unsigned char sample[MODEL_SAMPLE_SIZE] = {0};
    model_header(sample, MODEL_TYPE_SAMPLE, MODEL_SAMPLE_SIZE);
    memcpy(sample + 8, &ip, sizeof ip);
    model_put(ring, sample, sizeof sample);
}

static uint64_t drain(struct ring_model *ring, struct pf_ring_stats *stats)
{
    int status = pf_ring_drain(ring->data, MODEL_CAPACITY, ring->head,
                               &ring->tail, 4096, stats, NULL, NULL);
    PF_EXPECT(status == PF_RING_OK);
    return stats->lost;
}

static void test_final_drain_misses_trailing_losses(void)
{
    struct ring_model ring = {0};
    struct pf_ring_stats stats = {0};
    struct pf_loss_account account;

    /* 64 samples into a ring that holds 6: nothing drains until disable. */
    for (uint64_t i = 0; i < 64; ++i)
    {
        model_sample(&ring, 0x1000 + i);
    }
    uint64_t inband = drain(&ring, &stats);
    PF_EXPECT(stats.samples == 6);
    PF_EXPECT(ring.counted == 58);
    PF_EXPECT(inband == 0); /* the LOST record never fit */

    struct pf_loss_inputs final_only = {.ring_parsed = true,
                                        .inband_lost = inband};
    pf_loss_reconcile(&final_only, &account);
    PF_EXPECT(account.accounting == PF_LOSS_UNKNOWN && !account.lossless);

    struct pf_loss_inputs with_kernel = {
        .ring_parsed = true,
        .inband_lost = inband,
        .kernel_count_known = true,
        .kernel_lost = ring.counted,
    };
    pf_loss_reconcile(&with_kernel, &account);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE);
    PF_EXPECT(account.total_lost == 58 && account.unreported_lost == 58);
}

static void test_concurrent_drain_reports_losses(void)
{
    struct ring_model ring = {0};
    struct pf_ring_stats stats = {0};
    struct pf_loss_account account;

    /* Draining every 8 attempts lets the deferred LOST records fit. */
    for (uint64_t i = 0; i < 64; ++i)
    {
        model_sample(&ring, 0x2000 + i);
        if ((i % 8u) == 7u)
        {
            (void)drain(&ring, &stats);
        }
    }
    (void)drain(&ring, &stats);
    PF_EXPECT(stats.samples + ring.counted == 64);
    PF_EXPECT(stats.lost <= ring.counted);

    struct pf_loss_inputs inputs = {
        .ring_parsed = true,
        .inband_lost = stats.lost,
        .kernel_count_known = true,
        .kernel_lost = ring.counted,
    };
    pf_loss_reconcile(&inputs, &account);
    PF_EXPECT(account.accounting == PF_LOSS_COMPLETE);
    PF_EXPECT(account.total_lost == ring.counted);
    PF_EXPECT(account.unreported_lost == ring.counted - stats.lost);
    /* Only losses after the last record that fitted remain unreported. */
    PF_EXPECT(account.unreported_lost == ring.pending);
}

int main(void)
{
    test_decode();
    test_reconcile();
    test_unparsed_ring_is_unknown();
    test_throttling_and_unknown_records_block_lossless();
    test_final_drain_misses_trailing_losses();
    test_concurrent_drain_reports_losses();
    if (failure_count != 0u)
    {
        (void)fprintf(stderr, "PF_LOSS_FAIL:%u\n", failure_count);
        return EXIT_FAILURE;
    }
    (void)puts("PF_LOSS_PASS");
    return EXIT_SUCCESS;
}
