#include "checkpoint_firmware.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "P4_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                \
        }                                                           \
    } while (0)

enum
{
    HISTORY_STEPS = 8, /* offer/take/idle actions per history */
    ACTION_KINDS = 3, /* offer, take, idle */
    HISTORY_COUNT = 6561, /* ACTION_KINDS ** HISTORY_STEPS */
    ABORT_ATTEMPTS = 100,
    UNSET_SEQUENCE = 99 /* sentinel for outputs a failed take must not write */
};

/* Deliberate operational countermodel, not a racy C thread execution. Both
   producers see the same empty slot, then individually publish the same head.
   Do not fix: the CHECK asserts the loss. */
static void lost_update_countermodel(void)
{
    unsigned head = 0;
    unsigned tail = 0;
    unsigned slot[4] = {0};
    unsigned seen_a = head;
    unsigned seen_b = head;
    slot[seen_a] = 11;
    slot[seen_b] = 22;
    head = (seen_a + 1) % 4;
    head = (seen_b + 1) % 4;
    CHECK(head == 1 && tail == 0 && slot[tail] == 22); /* 11 was lost. */
}

/*
 * Real correction: separate single-producer rings, explicit source IDs.
 * Consumer round-robin defines service order, not physical arrival order.
 * Capacity 4 holds three samples, so sequences 3 and 4 of five are dropped.
 */
static void separate_rings(void)
{
    struct cpf_ring a;
    struct cpf_ring b;
    CHECK(cpf_ring_init(&a, 4) && cpf_ring_init(&b, 4));
    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(cpf_irq_push(&a, (struct cpf_sample){i, (uint16_t)(100 + i),
                                                   0}) == (i < 3));
        CHECK(cpf_irq_push(&b, (struct cpf_sample){i, (uint16_t)(200 + i),
                                                   0}) == (i < 3));
    }
    for (unsigned i = 0; i < 3; i++)
    {
        struct cpf_sample x;
        struct cpf_sample y;
        CHECK(cpf_take(&a, &x) && cpf_take(&b, &y));
        CHECK(x.sequence == i && y.sequence == i && x.value == 100 + i &&
              y.value == 200 + i);
    }
    struct cpf_sample out;
    CHECK(!cpf_take(&a, &out) && !cpf_take(&b, &out) && a.dropped == 2 &&
          b.dropped == 2);
}

static void two_producers(void)
{
    lost_update_countermodel();
    separate_rings();
}

/* Independent shifting-array model of the ring: values only, no indices. */
struct ring_model
{
    unsigned values[3];
    unsigned count;
    unsigned drops;
};

static void model_pop_front(struct ring_model *m)
{
    for (unsigned i = 1; i < m->count; i++)
    {
        m->values[i - 1] = m->values[i];
    }
    m->count--;
}

/* An offer is accepted while fewer than capacity - 1 items are queued. */
static void check_offer(struct cpf_ring *r, struct ring_model *m,
                        unsigned capacity, unsigned step)
{
    int expected = m->count < capacity - 1;
    CHECK(cpf_irq_push(r, (struct cpf_sample){step, (uint16_t)(100 + step),
                                              0}) == expected);
    if (expected)
    {
        m->values[m->count++] = step;
    }
    else
    {
        m->drops++;
    }
}

static void check_take(struct cpf_ring *r, struct ring_model *m)
{
    struct cpf_sample out = {UNSET_SEQUENCE, UNSET_SEQUENCE, UNSET_SEQUENCE};
    CHECK(cpf_take(r, &out) == (m->count != 0));
    if (m->count)
    {
        CHECK(out.sequence == m->values[0]);
        model_pop_front(m);
    }
    else
    {
        CHECK(out.sequence == UNSET_SEQUENCE);
    }
}

/* Replays one history, then drains the ring and compares with the model. */
static void check_history(unsigned capacity, unsigned code)
{
    struct cpf_ring r;
    struct ring_model model = {{0}, 0, 0};
    CHECK(cpf_ring_init(&r, capacity));
    for (unsigned step = 0; step < HISTORY_STEPS; step++, code /= ACTION_KINDS)
    {
        unsigned action = code % ACTION_KINDS;
        if (action == 0)
        {
            check_offer(&r, &model, capacity, step);
        }
        if (action == 1)
        {
            check_take(&r, &model);
        }
        CHECK(r.dropped == model.drops);
    }
    for (unsigned i = 0; i < model.count; i++)
    {
        struct cpf_sample out;
        CHECK(cpf_take(&r, &out) && out.sequence == model.values[i]);
    }
    struct cpf_sample out;
    CHECK(!cpf_take(&r, &out));
}

/*
 * All 3^8 offer/take/idle histories under physical capacities 4 and 2 (so 2
 * x 6561 = 13122 histories). No ISR data race is involved.
 */
static void histories(void)
{
    for (unsigned capacity = 2; capacity <= 4; capacity += 2)
    {
        for (unsigned code = 0; code < HISTORY_COUNT; code++)
        {
            check_history(capacity, code);
        }
    }
}

/*
 * A timed-out transmission stays retained (owned by DMA) through
 * ABORT_ATTEMPTS refused recoveries; only a recovery that reports both DMA
 * and UART quiescent releases it, and a late completion for the old
 * generation can then no longer touch the next operation.
 */
static void failed_abort(void)
{
    struct cpf_tx t;
    struct cp_message m = {.sequence = 3, .length = 1, .payload = {7}};
    cpf_tx_init(&t, 3, 2);
    CHECK(cpf_tx_begin(&t, &m, 1) == CPF_OK);
    unsigned g = t.generation;
    CHECK(cpf_tx_timeout(&t, g, 17) == CPF_PENDING);
    for (unsigned i = 0; i < ABORT_ATTEMPTS; i++)
    {
        CHECK(cpf_tx_recover(&t, g, 1, 1) == CPF_PENDING && t.retained &&
              t.retries == 0 && t.generation == g);
        CHECK(cpf_tx_begin(&t, &m, 1) == CPF_INVALID);
    }
    CHECK(cpf_tx_dma(&t, g, 0, 0, 1, 0) == CPF_INVALID &&
          t.retained); /* late TC cannot undo abort */
    CHECK(cpf_tx_timeout(&t, g, 99) == CPF_PENDING && t.fault_code == 17);
    CHECK(cpf_tx_recover(&t, g, 0, 1) == CPF_PENDING && t.retained);
    CHECK(cpf_tx_recover(&t, g, 0, 0) == CPF_OK && !t.retained &&
          t.generation == g + 1);
    CHECK(cpf_tx_begin(&t, &m, 1) == CPF_OK &&
          cpf_tx_dma(&t, g, 0, 0, 1, 0) == CPF_STALE && t.retained);
    CHECK(cpf_tx_timeout(&t, t.generation, 23) == CPF_PENDING);
    CHECK(cpf_tx_recover(&t, t.generation, 0, 0) == CPF_FAILED &&
          t.state == CPF_RETIRED);
}

/*
 * A 64-bit clock written as two 32-bit words under a sequence number: a
 * reader must reject every torn state around the low-word rollover.
 */
static void split_clock(void)
{
    struct cpf_clock c;
    CHECK(cpf_clock_init(&c));
    uint64_t out = 42;
    CHECK(cpf_clock_write(&c, UINT64_C(0xffffffff)));
    /* All four store cuts during the 32-bit low-word rollover. These atomic
       cut states model interleaving without invoking undefined data races. */
    atomic_store(&c.sequence, 3);
    CHECK(!cpf_clock_read(&c, 4, &out) && out == 42);
    atomic_store(&c.low, 0);
    CHECK(!cpf_clock_read(&c, 4, &out) && out == 42);
    CHECK((((uint64_t)atomic_load(&c.high) << 32) | atomic_load(&c.low)) == 0);
    atomic_store(&c.high, 1);
    CHECK(!cpf_clock_read(&c, 4, &out) && out == 42);
    atomic_store(&c.sequence, 4);
    CHECK(cpf_clock_read(&c, 1, &out) && out == UINT64_C(0x100000000));
    atomic_store(&c.sequence, UINT_MAX - 1);
    CHECK(!cpf_clock_write(&c, 0));
    uint16_t elapsed = 7;
    CHECK(!cpf_elapsed16(0, 32768, &elapsed) && elapsed == 7);
    CHECK(cpf_elapsed16(65530, 6, &elapsed) && elapsed == 12);
}

int main(void)
{
    histories();
    two_producers();
    failed_abort();
    split_clock();
    puts(
        "P4_VARIANTS_PASS histories=13122 producers=2 abort_attempts=100 clock_cuts=4");
}
