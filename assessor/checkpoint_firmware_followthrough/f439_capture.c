/* Reuse the already inspected F439 setup and strict providers explicitly.
   Its polling/DMA demonstration is retained under renamed entry points. */
#define cp_target_main p4_legacy_main
#define cp_pendsv p4_legacy_pendsv
#include "../checkpoint_depth/targets/f439.c"
#undef cp_target_main
#undef cp_pendsv
#include "capture_layout.h"
#include "halfduplex.h"
#include "register_rules.h"
#ifndef P4_TRACE
#define P4_TRACE 1
#endif
#ifndef P4_MODE
#define P4_MODE 1
#endif
#ifndef P4_RING
#define P4_RING 4
#endif
volatile struct p4_capture p4_capture;
static volatile uint32_t initialized = UINT32_C(0x13579bdf), zeroed;
static struct cpx_half_duplex half;
static volatile unsigned received;
static unsigned release_requested;

static uint32_t now(void)
{
    return REG(0xe0001004);
}

static unsigned received_and_dropped(unsigned *drops)
{
    unsigned saved;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(saved)::"memory");
    unsigned count = received;
    *drops = ring.dropped;
    __asm__ volatile("msr primask,%0" ::"r"(saved) : "memory");
    return count;
}

static void marker(unsigned bit, int set)
{
    if (P4_TRACE)
    {
        REG(0x40020018) = UINT32_C(1) << (bit + (set ? 0 : 16));
    }
}

static void direction(void *unused, int on)
{
    (void)unused;
    if (on)
    {
        release_requested = 0;
        REG(0x40020818) = 1;
    }
    else
    {
        release_requested =
            1; /* Hardware adapter releases after its own cycle guard. */
    }
}

static void release_direction(void)
{
    REG(0x40020818) = UINT32_C(1) << 16;
    release_requested = 0;
}

static int clock_setup(void)
{
    REG(0xe000edfc) |= UINT32_C(1) << 24;
    __asm__ volatile("dsb\nisb" ::: "memory");
    if (REG(0xe0001000) & (UINT32_C(1) << 25))
    {
        return 0;
    }
    REG(0xe0001004) = 0;
    REG(0xe0001000) |= 1;
    uint32_t before = now();
    for (unsigned i = 0; i < 100; i++)
    {
        __asm__ volatile("nop");
    }
    return now() != before;
}

void cp_pendsv(void)
{
    cp_fault();
}

void p4_exti0(void)
{
    if (!(REG(0x40013c14) & 1))
    {
        return;
    }
    REG(0x40013c14) =
        1; /* W1C, never read-modify-write unrelated pending bits. */
    unsigned index = received;
    if (index >= P4_EVENTS)
    {
        REG(0x40013c00) &= ~UINT32_C(1);
        return;
    }
    marker(3, 1);
    uint32_t tick = now();
    volatile struct p4_row *row = &p4_capture.rows[index];
    row->sequence = index;
    if (P4_TRACE)
    {
        row->isr_cycle = tick;
    }
    int accepted =
        cpf_irq_push(&ring, (struct cpf_sample){index, (uint16_t)(100 + index),
                                                (uint16_t)(tick >> 4)});
    if (P4_TRACE)
    {
        row->publish_cycle = now();
    }
    row->flags = accepted ? P4_ACCEPTED : P4_DROPPED;
    marker(4, 1);
    marker(4, 0);
    marker(3, 0);
    received = index + 1;
    if (received == P4_EVENTS)
    {
        REG(0x40013c00) &= ~UINT32_C(1);
    }
}

static void inputs_setup(void)
{
    REG(0x40023830) |=
        UINT32_C(4); /* GPIOC for declared teaching direction PC0. */
    REG(0x40023844) |= UINT32_C(1) << 14; /* SYSCFG */
    (void)REG(0x40023844);
    __asm__ volatile("dsb" ::: "memory");
    /* PA0 input, no pull. PA3..7 output markers; PA2 remains USART2 AF7. */
    REG(0x40020000) = (REG(0x40020000) & ~UINT32_C(0xffc3)) | UINT32_C(0x5540);
    REG(0x40020004) &= ~UINT32_C(0xf8);
    REG(0x40020008) &= ~UINT32_C(0xffc0);
    REG(0x4002000c) &= ~UINT32_C(0xffc3);
    REG(0x40020018) = UINT32_C(0xf8) << 16;
    REG(0x40020800) = (REG(0x40020800) & ~UINT32_C(3)) | 1;
    REG(0x40020804) &= ~UINT32_C(1);
    REG(0x40020808) &= ~UINT32_C(3);
    REG(0x4002080c) &= ~UINT32_C(3);
    release_direction();
    REG(0x40013808) &= ~UINT32_C(15); /* EXTI0 explicitly selects PA0. */
    REG(0x40013c00) &= ~UINT32_C(1);
    REG(0x40013c04) &= ~UINT32_C(1);
    REG(0x40013c08) |= 1;
    REG(0x40013c0c) &= ~UINT32_C(1);
    REG(0x40013c14) = 1;
    *(volatile uint8_t *)UINT32_C(0xe000e406) = 128; /* IRQ6 EXTI0 priority. */
    REG(0xe000e280) = UINT32_C(1) << 6;
    REG(0xe000e100) = UINT32_C(1) << 6;
    REG(0x40013c00) |= 1;
}

static int send(const struct cpf_sample *sample)
{
    struct cp_message message = {0};
    message.sequence = (uint16_t)sample->sequence;
    message.length = 8;
    for (unsigned i = 0; i < 4; i++)
    {
        message.payload[i] = (unsigned char)(sample->sequence >> (8 * i));
    }
    message.payload[4] = (unsigned char)sample->value;
    message.payload[5] = (unsigned char)(sample->value >> 8);
    message.payload[6] = (unsigned char)sample->tick;
    message.payload[7] = (unsigned char)(sample->tick >> 8);
    volatile struct p4_row *row = &p4_capture.rows[sample->sequence];
    struct cpf_tx *active = P4_MODE == 2 ? &half.tx : &tx;
    enum cpf_status begun =
        P4_MODE == 2 ? cpx_halfduplex_begin(&half, &message, 1000, direction, 0)
                     : cpf_tx_begin(&tx, &message, P4_MODE == 1);
    if (begun != CPF_OK)
    {
        row->flags |= P4_FAILED;
        p4_capture.first_fault = 3;
        p4_capture.fault_generation = active->generation;
        p4_capture.retained = (uint32_t)active->retained;
        return 0; /* Refused precondition: do not reset an unknown owner. */
    }
    row->generation = active->generation;
    row->flags |= P4_FRAMED;
    if (P4_TRACE)
    {
        row->frame_cycle = now();
    }
    marker(6, 1);
    marker(6, 0);
    if (P4_MODE == 1)
    {
        if (REG(0x400260a0) & 1)
        {
            goto abort_transfer;
        }
        REG(0x4002600c) = UINT32_C(0x3d) << 16;
        REG(0x400260a0) =
            (UINT32_C(4) << 25) | (UINT32_C(1) << 10) | (UINT32_C(1) << 6);
        REG(0x400260b4) = 0;
        REG(0x400260a4) = (uint32_t)tx.length;
        REG(0x400260a8) = UINT32_C(0x40004404);
        REG(0x400260ac) = (uint32_t)(uintptr_t)tx.wire;
        REG(0x40004400) = p4_tc_clear_word();
        __asm__ volatile("dmb" ::: "memory");
        REG(0x40004414) |= UINT32_C(0x80);
        REG(0x400260a0) |= 1;
    }
    uint32_t start = now(), tc_observed = 0;
    for (unsigned budget = 0; budget < 1000000; budget++)
    {
        uint32_t status = REG(0x40004400);
        enum cpf_status result;
        if (P4_MODE == 1)
        {
            unsigned enabled = REG(0x400260a0) & 1;
            uint32_t remaining = REG(0x400260a4), flags = REG(0x40026004);
            if (!enabled && !remaining && !(row->flags & P4_DMA_DONE))
            {
                row->flags |= P4_DMA_DONE;
                if (P4_TRACE)
                {
                    row->dma_cycle = now();
                }
                marker(7, 1);
                marker(7, 0);
            }
            result = cpf_tx_dma(active, active->generation, (int)enabled,
                                remaining, (status & 64) != 0,
                                (flags & (UINT32_C(0xd) << 16)) != 0);
        }
        else if (P4_MODE == 2)
        {
            result =
                release_requested
                    ? CPF_OK
                    : cpx_halfduplex_step(&half, (uint16_t)(now() >> 4),
                                          (status & 128) != 0,
                                          (status & 64) != 0, write_byte, 0);
        }
        else
        {
            result =
                cpf_tx_poll(active, active->generation, (status & 128) != 0,
                            (status & 64) != 0, write_byte, 0);
        }
        /* TC is a sampled status observation, not the exact external stop-bit edge. */
        if ((P4_MODE == 2 ? half.holding : result == CPF_OK) &&
            !(row->flags & P4_WIRE_DONE))
        {
            row->flags |= P4_WIRE_DONE;
            if (P4_TRACE)
            {
                row->tc_cycle = now();
            }
            marker(5, 1);
            marker(5, 0);
            tc_observed = now();
        }
        if (result == CPF_OK)
        {
            if (P4_MODE == 2 &&
                (!release_requested || (uint32_t)(now() - tc_observed) < 16000))
            {
                continue;
            }
            if (P4_MODE == 2)
            {
                release_direction();
            }
            REG(0x40004414) &= ~UINT32_C(0x80);
            if (P4_MODE == 2 || cpf_tx_next(active) == CPF_OK)
            {
                return 1;
            }
            row->flags |= P4_FAILED;
            p4_capture.first_fault = 4;
            p4_capture.fault_generation = active->generation;
            p4_capture.retained = (uint32_t)active->retained;
            return 0;
        }
        if (result == CPF_FAILED ||
            (uint32_t)(now() - start) >= UINT32_C(1600000))
        {
            break;
        }
    }
abort_transfer:
    (void)cpf_tx_timeout(active, active->generation, 2);
    row->flags |= P4_FAILED;
    REG(0x40004414) &= ~UINT32_C(0x80);
    REG(0x400260a0) &= ~UINT32_C(1);
    if (wait_mask(0x400260a0, 1, 0))
    {
        REG(0x4000440c) &= ~(UINT32_C(1) << 13);
        (void)cpf_tx_recover(active, active->generation, 0,
                             (REG(0x4000440c) & (UINT32_C(1) << 13)) != 0);
        if (!active->retained && P4_MODE == 2)
        {
            release_direction();
        }
    }
    p4_capture.first_fault = active->fault_code ? active->fault_code : 2;
    p4_capture.fault_generation =
        active->fault_code ? active->fault_generation : active->generation;
    p4_capture.retained = (uint32_t)active->retained;
    p4_capture.retries = active->retries;
    return 0; /* Safe stopped experiment. No automatic partial-frame replay. */
}

void cp_target_main(void)
{
    if (initialized != UINT32_C(0x13579bdf) || zeroed || received ||
        p4_capture.magic)
    {
        cp_fault();
    }
    if (!cpf_ring_init(&ring, P4_RING) || !setup() || !clock_setup())
    {
        cp_fault();
    }
    cpf_tx_init(&tx, 128, 2);
    p4_capture.magic = P4_CAPTURE_MAGIC;
    p4_capture.version = 1;
    p4_capture.trace_enabled = P4_TRACE;
    p4_capture.mode = P4_MODE;
    p4_capture.ring_capacity = P4_RING;
    p4_capture.state = 1;
    p4_capture.clock_start = now();
    inputs_setup();
    __asm__ volatile("cpsie i" ::: "memory");
    unsigned completed = 0;
    int failed = 0;
    while ((uint32_t)(now() - p4_capture.clock_start) < UINT32_C(32000000))
    {
        struct cpf_sample sample;
        if (cpf_take(&ring, &sample))
        {
            if (!send(&sample))
            {
                failed = 1;
                break;
            }
            ++completed;
        }
        unsigned dropped;
        if (received_and_dropped(&dropped) == P4_EVENTS &&
            completed + dropped == P4_EVENTS)
        {
            break;
        }
    }
    __asm__ volatile("cpsid i" ::: "memory");
    REG(0x40013c00) &= ~UINT32_C(1);
    p4_capture.clock_end = now();
    p4_capture.received = received;
    p4_capture.dropped = ring.dropped;
    p4_capture.accepted = received - ring.dropped;
    p4_capture.completed = completed;
    p4_capture.state = failed ? 3 : 2;
    __asm__ volatile("dsb" ::: "memory");
    for (;;)
    {
    }
}
