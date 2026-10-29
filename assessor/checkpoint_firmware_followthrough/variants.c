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

/* Deliberate operational countermodel, not a racy C thread execution. Both
   producers see the same empty slot, then individually publish the same head. */
static void two_producers(void)
{
    unsigned head = 0, tail = 0, slot[4] = {0}, seen_a = head, seen_b = head;
    slot[seen_a] = 11;
    slot[seen_b] = 22;
    head = (seen_a + 1) % 4;
    head = (seen_b + 1) % 4;
    CHECK(head == 1 && tail == 0 && slot[tail] == 22); /* 11 was lost. */
    /* Real correction: separate single-producer rings, explicit source IDs.
       Consumer round-robin defines service order, not physical arrival order. */
    struct cpf_ring a, b;
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
        struct cpf_sample x, y;
        CHECK(cpf_take(&a, &x) && cpf_take(&b, &y));
        CHECK(x.sequence == i && y.sequence == i && x.value == 100 + i &&
              y.value == 200 + i);
    }
    struct cpf_sample out;
    CHECK(!cpf_take(&a, &out) && !cpf_take(&b, &out) && a.dropped == 2 &&
          b.dropped == 2);
}

static void histories(void)
{
    /* Independent shifting-array oracle: all 3^8 offer/take/idle histories,
       identical workload under physical capacities4 and2. No ISR data race. */
    for (unsigned capacity = 2; capacity <= 4; capacity += 2)
    {
        for (unsigned code = 0; code < 6561; code++)
        {
            struct cpf_ring r;
            CHECK(cpf_ring_init(&r, capacity));
            unsigned model[3] = {0}, count = 0, drops = 0, n = code;
            for (unsigned step = 0; step < 8; step++, n /= 3)
            {
                unsigned action = n % 3;
                if (action == 0)
                {
                    int expected = count < capacity - 1;
                    CHECK(cpf_irq_push(&r, (struct cpf_sample){
                                               step, (uint16_t)(100 + step),
                                               0}) == expected);
                    if (expected)
                    {
                        model[count++] = step;
                    }
                    else
                    {
                        drops++;
                    }
                }
                if (action == 1)
                {
                    struct cpf_sample out = {99, 99, 99};
                    CHECK(cpf_take(&r, &out) == (count != 0));
                    if (count)
                    {
                        CHECK(out.sequence == model[0]);
                        for (unsigned i = 1; i < count; i++)
                        {
                            model[i - 1] = model[i];
                        }
                        count--;
                    }
                    else
                    {
                        CHECK(out.sequence == 99);
                    }
                }
                CHECK(r.dropped == drops);
            }
            for (unsigned i = 0; i < count; i++)
            {
                struct cpf_sample out;
                CHECK(cpf_take(&r, &out) && out.sequence == model[i]);
            }
            struct cpf_sample out;
            CHECK(!cpf_take(&r, &out));
        }
    }
}

static void failed_abort(void)
{
    struct cpf_tx t;
    struct cp_message m = {.sequence = 3, .length = 1, .payload = {7}};
    cpf_tx_init(&t, 3, 2);
    CHECK(cpf_tx_begin(&t, &m, 1) == CPF_OK);
    unsigned g = t.generation;
    CHECK(cpf_tx_timeout(&t, g, 17) == CPF_PENDING);
    for (unsigned i = 0; i < 100; i++)
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
