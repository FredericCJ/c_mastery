/* Appended to the explicit completed startup translation unit. */
#include "queues.h"
#include "locks.c"
#include "queues.c"
#ifndef SQ_COARSE
#define SQ_COARSE 0
#endif
#ifndef SQ_GUEST_MUTANT
#define SQ_GUEST_MUTANT 0
#endif
static struct sq_world sqg_world;
static struct sq_handle sqg_ids[8];
static uint64_t *sqg_sp[8], *sqg_idle[4];
static unsigned char sqg_stack[8][8192] ALIGN(16);
static unsigned sqg_current[4], sqg_requested[8], sqg_executing[8],
    sqg_actions[8], sqg_seen[8];
static unsigned sqg_go, sqg_phase, sqg_done, sqg_parked[4], sqg_finished[4];
static unsigned sqg_count[4], sqg_switches[4], sqg_empty[4];
static uint64_t sqg_active[4], sqg_elapsed[4], sqg_idle_ticks[4];
static struct sq_event sqg_events[4][128];
static struct sl_stats sqg_stats[4];

static void sqg_fail(const char *reason)
{
    text("SQ_GUEST_CHECK:");
    text(reason);
    text("\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}

static uint64_t sqg_ticks(void)
{
    unsigned low, high;
    __asm__ volatile("lfence;rdtsc" : "=a"(low), "=d"(high)::"memory");
    return (uint64_t)high << 32 | low;
}

static void sqg_log(unsigned c, struct sq_event e)
{
    if (c != e.actor || sqg_count[c] == 128)
        sqg_fail("trace-capacity-or-actor");
    sqg_events[c][sqg_count[c]++] = e;
}

static void sqg_task(void)
{
    unsigned slot = sqg_current[cmk2_cpu_index()];
    volatile uint64_t canary[8];
    for (unsigned j = 0; j < 8; j++)
        canary[j] = UINT64_C(0xabc000) + slot * 16 + j;
    for (unsigned iteration = 0; iteration < 7; iteration++)
    {
        unsigned c = cmk2_cpu_index();
        if (sqg_current[c] != slot ||
            atomic_load(&sqg_world.task[slot].state) != SQ_RUNNING ||
            !sqg_executing[slot])
            sqg_fail("context-without-exclusive-owner");
        for (unsigned j = 0; j < 8; j++)
            if (canary[j] != UINT64_C(0xabc000) + slot * 16 + j)
                sqg_fail("saved-stack-canary");
        sqg_seen[slot] |= 1u << c;
        sqg_actions[slot]++;
        sqg_requested[slot] = iteration == 2
                                  ? SQ_BLOCKED
                                  : (iteration == 6 ? SQ_EXITED : SQ_READY);
        cmk2_switch(&sqg_sp[slot], sqg_idle[c]);
    }
    sqg_fail("exited-context-resumed");
}

static int sqg_step(unsigned c)
{
    struct sq_handle id;
    struct sq_event event;
    int result = sq_dispatch(&sqg_world, c, &id, &event, &sqg_stats[c]);
    if (result == SQ_IDLE)
        return 0;
    if (result != SQ_OK)
        sqg_fail("dispatch-admission");
    sqg_log(c, event);
    sqg_current[c] = id.slot;
    if (__atomic_exchange_n(&sqg_executing[id.slot], 1, __ATOMIC_ACQ_REL))
        sqg_fail("duplicate-running-stack");
    if (SQ_GUEST_MUTANT == 1)
    {
        if (sq_finish(&sqg_world, c, id, SQ_READY, &event, &sqg_stats[c]) !=
            SQ_OK)
            sqg_fail("early-finish-control");
    }
    if (atomic_load(&sqg_world.task[id.slot].state) != SQ_RUNNING)
        sqg_fail("published-context-before-save");
    uint64_t begin = sqg_ticks();
    cmk2_switch(&sqg_idle[c], sqg_sp[id.slot]);
    sqg_active[c] += sqg_ticks() - begin;
    sqg_switches[c] += 2;
    __atomic_store_n(&sqg_executing[id.slot], 0, __ATOMIC_RELEASE);
    if (sq_finish(&sqg_world, c, id, sqg_requested[id.slot], &event,
                  &sqg_stats[c]) != SQ_OK)
        sqg_fail("saved-context-finish");
    sqg_log(c, event);
    if (sqg_requested[id.slot] == SQ_EXITED)
        __atomic_fetch_add(&sqg_done, 1, __ATOMIC_RELEASE);
    return 1;
}

static void sqg_run(unsigned c)
{
    uint64_t begin = sqg_ticks();
    for (;;)
    {
        if (!sqg_step(c))
        {
            if (atomic_load(&sqg_world.task[2 * c].state) != SQ_BLOCKED ||
                atomic_load(&sqg_world.task[2 * c + 1].state) != SQ_BLOCKED)
                sqg_fail("last-task-block-state");
            sqg_empty[c]++;
            break;
        }
    }
    __atomic_store_n(&sqg_parked[c], 1, __ATOMIC_RELEASE);
    if (c == 0)
    {
        for (unsigned i = 1; i < topology.count; i++)
            while (!__atomic_load_n(&sqg_parked[i], __ATOMIC_ACQUIRE))
                pause_cpu();
        if (!sq_invariant(&sqg_world))
            sqg_fail("blocked-snapshot");
        for (unsigned slot = 0; slot < 2 * topology.count; slot++)
        {
            struct sq_event e;
            if (sq_wake(&sqg_world, 0, slot / 2, sqg_ids[slot], &e,
                        &sqg_stats[0]) != SQ_OK)
                sqg_fail("remote-wake");
            sqg_log(0, e);
        }
        if (topology.count > 1)
        {
            struct sq_transfer t;
            struct sq_event e;
            if (sq_transfer_begin(&sqg_world, 0, 0, 1, sqg_ids[0], &t, &e,
                                  &sqg_stats[0]) != SQ_OK)
                sqg_fail("saved-transfer-begin");
            sqg_log(0, e);
            if (sq_transfer_end(&sqg_world, 0, t, 1, &e, &sqg_stats[0]) !=
                SQ_OK)
                sqg_fail("saved-transfer-end");
            sqg_log(0, e);
        }
        if (!sq_invariant(&sqg_world))
            sqg_fail("transfer-snapshot");
        __atomic_store_n(&sqg_phase, 1, __ATOMIC_RELEASE);
    }
    else
        while (!__atomic_load_n(&sqg_phase, __ATOMIC_ACQUIRE))
            pause_cpu();
    while (__atomic_load_n(&sqg_done, __ATOMIC_ACQUIRE) < 2 * topology.count)
    {
        uint64_t idle = sqg_ticks();
        if (!sqg_step(c))
        {
            pause_cpu();
            sqg_idle_ticks[c] += sqg_ticks() - idle;
        }
    }
    sqg_elapsed[c] = sqg_ticks() - begin;
    __atomic_store_n(&sqg_finished[c], 1, __ATOMIC_RELEASE);
}

static void sqg_worker(unsigned c)
{
    while (!__atomic_load_n(&sqg_go, __ATOMIC_ACQUIRE))
        pause_cpu();
    sqg_run(c);
    for (;;)
        __asm__ volatile("cli;hlt");
}

void sqg_bsp(void)
{
    if (sq_init(&sqg_world, topology.count, 4, SQ_COARSE) != SQ_OK)
        sqg_fail("world-init");
    for (unsigned slot = 0; slot < 2 * topology.count; slot++)
    {
        struct sq_event e;
        if (sq_create(&sqg_world, 0, slot / 2, slot, &sqg_ids[slot], &e) !=
            SQ_OK)
            sqg_fail("task-create");
        sqg_log(0, e);
        uint64_t *sp = (uint64_t *)(sqg_stack[slot] + 8192);
        *--sp = 0;
        *--sp = (uintptr_t)sqg_task;
        for (unsigned i = 0; i < 6; i++)
            *--sp = 0;
        sqg_sp[slot] = sp;
    }
    __atomic_store_n(&sqg_go, 1, __ATOMIC_RELEASE);
    sqg_run(0);
    for (unsigned c = 1; c < topology.count; c++)
        while (!__atomic_load_n(&sqg_finished[c], __ATOMIC_ACQUIRE))
            pause_cpu();
    if (!sq_invariant(&sqg_world))
        sqg_fail("final-snapshot");
    for (unsigned slot = 0; slot < 2 * topology.count; slot++)
    {
        if (sqg_actions[slot] != 7 || sqg_executing[slot] ||
            atomic_load(&sqg_world.task[slot].state) != SQ_EXITED)
            sqg_fail("task-completion");
        if (slot == 0 && topology.count > 1 && sqg_seen[slot] != 3)
            sqg_fail("context-migration-missing");
    }
    for (unsigned c = 0; c < topology.count; c++)
    {
        text("SQ_CPU actor=");
        number(c);
        text(" apic=");
        number(topology.ids[c]);
        text(" switches=");
        number(sqg_switches[c]);
        text(" empty=");
        number(sqg_empty[c]);
        text(" active=");
        ss_u64(sqg_active[c]);
        text(" idle=");
        ss_u64(sqg_idle_ticks[c]);
        text(" elapsed=");
        ss_u64(sqg_elapsed[c]);
        text("\n");
        for (unsigned seq = 0; seq < sqg_count[c]; seq++)
        {
            struct sq_event *e = &sqg_events[c][seq];
            text("SQ_EVENT actor=");
            number(c);
            text(" seq=");
            number(seq);
            text(" slot=");
            number(e->slot);
            text(" generation=");
            ss_u64(e->generation);
            text(" revision=");
            ss_u64(e->revision);
            text(" from=");
            number(e->from);
            text(" to=");
            number(e->to);
            text(" source=");
            number(e->source);
            text(" destination=");
            number(e->destination);
            text("\n");
        }
    }
    text("SQ_GUEST_PASS:saved-context-percpu-queues coarse=");
    number(SQ_COARSE);
    text(" tasks=");
    number(2 * topology.count);
    text("\n");
    unsigned value = 16;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}
