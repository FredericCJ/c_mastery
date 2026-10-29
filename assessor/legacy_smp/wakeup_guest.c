#include "wakeup.h"
#include "locks.c"
#include "queues.c"
#include "wakeup.c"
#ifndef SW_GUEST_MUTANT
#define SW_GUEST_MUTANT 0
#endif
enum
{
    SWG_ROUNDS = 24
};

extern void swg_irq(void), swg_sleep(void), swg_bad_sleep(void);
extern unsigned char swg_after_halt[], swg_after_bad_halt[];
static struct sw_world swg_world;
static struct sq_handle swg_ids[4];
static uint64_t *swg_sp[4], *swg_idle;
static unsigned char swg_stack[4][8192] ALIGN(16);
static unsigned swg_go, swg_ready, swg_command, swg_round, swg_stage,
    swg_published, swg_done, swg_finished, swg_busy, swg_bad, swg_rescue;
static unsigned swg_current, swg_actions[4], swg_interrupts, swg_halted,
    swg_early, swg_notify_count;
static unsigned swg_background[4];
static uint64_t swg_publish[SWG_ROUNDS], swg_notify[SWG_ROUNDS],
    swg_observed[SWG_ROUNDS], swg_dispatch[SWG_ROUNDS],
    swg_irq_time[SWG_ROUNDS];
static unsigned swg_irq_count[SWG_ROUNDS], swg_halt_count[SWG_ROUNDS],
    swg_count[SWG_ROUNDS];
static struct sl_stats swg_stats[2];

static void swg_fail(const char *s)
{
    text("SW_GUEST_CHECK:");
    text(s);
    text("\n");
    unsigned v = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(v));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

static uint64_t swg_ticks(void)
{
    unsigned lo, hi;
    __asm__ volatile("lfence;rdtsc" : "=a"(lo), "=d"(hi)::"memory");
    return (uint64_t)hi << 32 | lo;
}

static void swg_delay(void)
{
    uint64_t begin = swg_ticks();
    while (swg_ticks() - begin < UINT64_C(2000000))
    {
        pause_cpu();
    }
}

static unsigned swg_mode(unsigned round)
{
    return SW_GUEST_MUTANT == 2 ? 2 : round % 4;
}

static unsigned swg_batch(unsigned round)
{
    return swg_mode(round) == 3 || round & 4 ? 4 : 1;
}

static void swg_send(void)
{
    __asm__ volatile("mfence" ::: "memory");
    ipi(topology.ids[1], 245);
}

static int swg_ring(void *unused, unsigned cpu)
{
    (void)unused;
    if (cpu != 1)
    {
        swg_fail("unexpected-notification-destination");
    }
    unsigned round = __atomic_load_n(&swg_round, __ATOMIC_ACQUIRE);
    if (!swg_notify[round])
    {
        swg_notify[round] = swg_ticks();
    }
    swg_notify_count++;
    unsigned before = __atomic_load_n(&swg_interrupts, __ATOMIC_ACQUIRE);
    swg_send();
    if (SW_GUEST_MUTANT == 2 &&
        !atomic_load_explicit(&swg_world.work[1], memory_order_acquire))
    {
        for (unsigned spin = 0;
             __atomic_load_n(&swg_interrupts, __ATOMIC_ACQUIRE) == before;
             spin++)
        {
            if (spin == 100000000)
            {
                swg_fail("early-interrupt-not-delivered");
            }
            pause_cpu();
        }
    }
    return 1;
}

static void swg_ack(void *unused, unsigned cpu)
{
    (void)unused;
    (void)
        cpu; /* Fixed IPI EOI is in ISR; software level clear is in the core. */
}

void swg_interrupt(uint64_t *frame)
{
    if (cmk2_cpu_index() != 1 || frame[15] != 245)
    {
        swg_fail("interrupt-route");
    }
    unsigned round = __atomic_load_n(&swg_round, __ATOMIC_ACQUIRE);
    if (!swg_irq_time[round])
    {
        swg_irq_time[round] = swg_ticks();
    }
    if (frame[17] == (uintptr_t)swg_after_halt ||
        frame[17] == (uintptr_t)swg_after_bad_halt)
    {
        swg_halt_count[round]++;
        swg_halted++;
    }
    if (SW_GUEST_MUTANT == 1 &&
        __atomic_load_n(&swg_rescue, __ATOMIC_ACQUIRE) &&
        frame[17] != (uintptr_t)swg_after_bad_halt)
    {
        swg_fail("rescue-not-at-halt");
    }
    if (!atomic_load_explicit(&swg_world.work[1], memory_order_acquire))
    {
        swg_early++;
    }
    swg_irq_count[round]++;
    lapic[0xb0 / 4] = 0;
    __atomic_add_fetch(&swg_interrupts, 1, __ATOMIC_RELEASE);
}

static void swg_task(void)
{
    unsigned slot = swg_current;
    volatile uint64_t marker = UINT64_C(0x12340000) + slot;
    /* Establish a genuine saved blocked context before the first remote wake. */
    cmk2_switch(&swg_sp[slot], swg_idle);
    for (;;)
    {
        if (swg_current != slot || marker != UINT64_C(0x12340000) + slot)
        {
            swg_fail("context-identity");
        }
        unsigned round = __atomic_load_n(&swg_round, __ATOMIC_ACQUIRE);
        if (swg_mode(round) == 3 && slot == 0)
        {
            __atomic_store_n(&swg_busy, round + 1, __ATOMIC_RELEASE);
            while (__atomic_load_n(&swg_published, __ATOMIC_ACQUIRE) !=
                   round + 1)
            {
                pause_cpu();
            }
        }
        swg_actions[slot]++;
        marker += 7;
        marker -= 7;
        cmk2_switch(&swg_sp[slot], swg_idle);
    }
}

static int swg_step(unsigned round, int boot)
{
    struct sq_handle id;
    struct sq_event event;
    int r = sw_dispatch(&swg_world, 1, &id, &event, &swg_stats[1]);
    if (r == SQ_IDLE)
    {
        return 0;
    }
    if (r != SQ_OK)
    {
        swg_fail("dispatch-refused");
    }
    swg_current = id.slot;
    if (!boot && !swg_dispatch[round])
    {
        swg_dispatch[round] = swg_ticks();
    }
    cmk2_switch(&swg_idle, swg_sp[id.slot]);
    if (sw_finish(&swg_world, 1, id, SQ_BLOCKED, &event, &swg_stats[1]) !=
        SQ_OK)
    {
        swg_fail("saved-block-refused");
    }
    if (!boot)
    {
        swg_count[round]++;
    }
    return 1;
}

static void swg_receiver(void)
{
    uint64_t address = (uintptr_t)swg_irq;
    cpus[1].idt[245] = (struct gate){
        address & 65535,           8, 1, 0x8e, (address >> 16) & 65535,
        (uint32_t)(address >> 32), 0};
    for (unsigned i = 0; i < 4; i++)
    {
        if (!swg_step(0, 1))
        {
            swg_fail("initial-block");
        }
    }
    if (sw_prepare_idle(&swg_world, 1, 1, &swg_stats[1]) != SQ_IDLE)
    {
        swg_fail("initial-empty");
    }
    __atomic_store_n(&swg_ready, 1, __ATOMIC_RELEASE);
    for (unsigned round = 0; round < SWG_ROUNDS; round++)
    {
        while (__atomic_load_n(&swg_command, __ATOMIC_ACQUIRE) != round + 1)
        {
            pause_cpu();
        }
        unsigned mode = swg_mode(round), batch = swg_batch(round),
                 before = __atomic_load_n(&swg_interrupts, __ATOMIC_ACQUIRE);
        __asm__ volatile("cli" ::: "memory");
        if (mode != 0 && mode != 3 &&
            sw_prepare_idle(&swg_world, 1, 1, &swg_stats[1]) != SQ_IDLE)
        {
            swg_fail("masked-check-not-empty");
        }
        __atomic_store_n(&swg_stage, round + 1, __ATOMIC_RELEASE);
        if (mode == 0 || mode == 1)
        {
            while (__atomic_load_n(&swg_published, __ATOMIC_ACQUIRE) !=
                   round + 1)
            {
                pause_cpu();
            }
        }
        if (mode == 0)
        {
            if (sw_prepare_idle(&swg_world, 1, 1, &swg_stats[1]) != SW_BUSY)
            {
                swg_fail("before-check-work-lost");
            }
        }
        else if (mode == 1 || mode == 2)
        {
            if (SW_GUEST_MUTANT == 1 && mode == 1)
            {
                __asm__ volatile("sti;nop" ::: "memory");
                while (__atomic_load_n(&swg_interrupts, __ATOMIC_ACQUIRE) ==
                       before)
                {
                    pause_cpu();
                }
                __atomic_store_n(&swg_bad, 1, __ATOMIC_RELEASE);
                swg_bad_sleep();
                if (!__atomic_load_n(&swg_rescue, __ATOMIC_ACQUIRE) ||
                    !atomic_load(&swg_world.work[1]))
                {
                    swg_fail("missing-controlled-stranded-work");
                }
                swg_fail("missed-wakeup-until-rescue");
            }
            swg_sleep();
        }
        __asm__ volatile("sti" ::: "memory");
        while (swg_count[round] < batch)
        {
            if (!swg_step(round, 0))
            {
                pause_cpu();
            }
        }
        __asm__ volatile("cli" ::: "memory");
        if (sw_prepare_idle(&swg_world, 1, 1, &swg_stats[1]) != SQ_IDLE)
        {
            swg_fail("batch-not-fully-drained");
        }
        __atomic_store_n(&swg_done, round + 1, __ATOMIC_RELEASE);
    }
    __atomic_store_n(&swg_finished, 1, __ATOMIC_RELEASE);
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

static void swg_worker(unsigned c)
{
    while (!__atomic_load_n(&swg_go, __ATOMIC_ACQUIRE))
    {
        pause_cpu();
    }
    if (c == 1)
    {
        swg_receiver();
    }
    while (!__atomic_load_n(&swg_finished, __ATOMIC_ACQUIRE))
    {
        __atomic_add_fetch(&swg_background[c], 1, __ATOMIC_RELAXED);
        pause_cpu();
    }
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

void swg_bsp(void)
{
    if (topology.count < 2)
    {
        swg_fail("remote-experiment-needs-two-CPUs");
    }
    if (sw_init(&swg_world, topology.count, 4,
                (struct sw_port){swg_ring, swg_ack, 0}) != SQ_OK)
    {
        swg_fail("init");
    }
    for (unsigned slot = 0; slot < 4; slot++)
    {
        struct sq_event event;
        if (sw_create(&swg_world, 0, 1, slot, &swg_ids[slot], &event) != SQ_OK)
        {
            swg_fail("create");
        }
        uint64_t *sp = (uint64_t *)(swg_stack[slot] + 8192);
        *--sp = 0;
        *--sp = (uintptr_t)swg_task;
        for (unsigned k = 0; k < 6; k++)
        {
            *--sp = 0;
        }
        swg_sp[slot] = sp;
    }
    __atomic_store_n(&swg_go, 1, __ATOMIC_RELEASE);
    while (!__atomic_load_n(&swg_ready, __ATOMIC_ACQUIRE))
    {
        pause_cpu();
    }
    for (unsigned round = 0; round < SWG_ROUNDS; round++)
    {
        unsigned mode = swg_mode(round), batch = swg_batch(round);
        __atomic_store_n(&swg_round, round, __ATOMIC_RELEASE);
        __atomic_store_n(&swg_command, round + 1, __ATOMIC_RELEASE);
        while (__atomic_load_n(&swg_stage, __ATOMIC_ACQUIRE) != round + 1)
        {
            pause_cpu();
        }
        if (mode == 2)
        {
            swg_delay();
        }
        swg_publish[round] = swg_ticks();
        for (unsigned slot = 0; slot < batch; slot++)
        {
            struct sq_event event;
            if (sw_wake(&swg_world, 0, 1, swg_ids[slot], &event,
                        &swg_stats[0]) != SQ_OK)
            {
                swg_fail("remote-publication");
            }
            if (mode == 3 && slot == 0)
            {
                while (__atomic_load_n(&swg_busy, __ATOMIC_ACQUIRE) !=
                       round + 1)
                {
                    pause_cpu();
                }
                if (sw_wake(&swg_world, 0, 1, swg_ids[slot], &event,
                            &swg_stats[0]) != SQ_DUPLICATE)
                {
                    swg_fail("running-wake-duplicated");
                }
            }
        }
        __atomic_store_n(&swg_published, round + 1, __ATOMIC_RELEASE);
        if (SW_GUEST_MUTANT == 1 && mode == 1)
        {
            while (!__atomic_load_n(&swg_bad, __ATOMIC_ACQUIRE))
            {
                pause_cpu();
            }
            swg_delay();
            __atomic_store_n(&swg_rescue, 1, __ATOMIC_RELEASE);
            swg_send();
        }
        for (unsigned spin = 0;
             __atomic_load_n(&swg_done, __ATOMIC_ACQUIRE) != round + 1; spin++)
        {
            if (spin == 200000000)
            {
                swg_fail("dispatch-observation-timeout");
            }
            pause_cpu();
        }
        swg_observed[round] = swg_ticks();
    }
    while (!__atomic_load_n(&swg_finished, __ATOMIC_ACQUIRE))
    {
        pause_cpu();
    }
    if (!sq_invariant(&swg_world.queues) || atomic_load(&swg_world.work[1]) ||
        swg_world.bell[1])
    {
        swg_fail("final-work-state");
    }
    if (SW_GUEST_MUTANT == 2)
    {
        if (!swg_early)
        {
            swg_fail("early-order-control-not-exposed");
        }
        swg_fail("IPI-before-publication");
    }
    if (!swg_halted)
    {
        swg_fail("no-actual-halt-resume");
    }
    for (unsigned c = 2; c < topology.count; c++)
    {
        if (!__atomic_load_n(&swg_background[c], __ATOMIC_RELAXED))
        {
            swg_fail("missing-declared-load");
        }
    }
    for (unsigned r = 0; r < SWG_ROUNDS; r++)
    {
        text("SW_SAMPLE round=");
        number(r);
        text(" mode=");
        number(swg_mode(r));
        text(" batch=");
        number(swg_batch(r));
        text(" publication=");
        ss_u64(swg_publish[r]);
        text(" notification=");
        ss_u64(swg_notify[r]);
        text(" observed=");
        ss_u64(swg_observed[r]);
        text(" dispatch=");
        ss_u64(swg_dispatch[r]);
        text(" irq=");
        ss_u64(swg_irq_time[r]);
        text(" irqs=");
        number(swg_irq_count[r]);
        text(" halt=");
        number(swg_halt_count[r]);
        text(" delivered=");
        number(swg_count[r]);
        text("\n");
    }
    text("SW_GUEST_PASS:durable-work-idle-IPI cpus=");
    number(topology.count);
    text(" rounds=24 halted=");
    number(swg_halted);
    text(" notifications=");
    number(swg_notify_count);
    text("\n");
    unsigned v = 16;
    __asm__ volatile("outl %0,$0xf4" ::"a"(v));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
