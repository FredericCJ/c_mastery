/* Appended to the explicitly completed startup translation unit. */
#include "balance.h"
#include "locks.c"
#include "queues.c"
#include "wakeup.c"
#include "balance.c"
#ifndef SB_POLICY
#define SB_POLICY 1
#endif
#ifndef SB_GUEST_MUTANT
#define SB_GUEST_MUTANT 0
#endif
static struct sb_world sbg_world;
static struct sq_handle sbg_ids[4];
static uint64_t *sbg_sp[4], *sbg_idle[4];
static unsigned char sbg_stack[4][8192] ALIGN(16);
static unsigned sbg_current[4], sbg_requested[4], sbg_executing[4],
    sbg_actions[4], sbg_seen[4];
static unsigned sbg_go, sbg_phase, sbg_done, sbg_parked[4], sbg_finished[4],
    sbg_blocked;
static unsigned sbg_count[4], sbg_switches[4], sbg_rings[4];
static uint64_t sbg_elapsed[4], sbg_active[4];
static struct sq_event sbg_events[4][128];
static struct sl_stats sbg_stats[4];

static void sbg_fail(const char *reason)
{
    text("SB_GUEST_CHECK:");
    text(reason);
    text("\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

static uint64_t sbg_ticks(void)
{
    unsigned low, high;
    __asm__ volatile("lfence;rdtsc" : "=a"(low), "=d"(high)::"memory");
    return (uint64_t)high << 32 | low;
}

/* This comparison deliberately polls. The separate T53.07 adapter exercises
 * actual sleeping destinations and IPIs; these callbacks count notifications. */
static int sbg_ring(void *unused, unsigned c)
{
    (void)unused;
    sbg_rings[c]++;
    return 1;
}

static void sbg_ack(void *unused, unsigned c)
{
    (void)unused;
    (void)c;
}

static void sbg_log(unsigned c, struct sq_event e)
{
    if (c != e.actor || sbg_count[c] == 128)
    {
        sbg_fail("trace-capacity-or-actor");
    }
    sbg_events[c][sbg_count[c]++] = e;
}

static void sbg_task(void)
{
    unsigned slot = sbg_current[cmk2_cpu_index()];
    volatile uint64_t canary[8];
    for (unsigned j = 0; j < 8; j++)
    {
        canary[j] = UINT64_C(0xabd000) + slot * 16 + j;
    }
    for (unsigned iteration = 0; iteration < 7; iteration++)
    {
        unsigned c = cmk2_cpu_index();
        if (sbg_current[c] != slot ||
            atomic_load(&sbg_world.wake.queues.task[slot].state) !=
                SQ_RUNNING ||
            !sbg_executing[slot])
        {
            sbg_fail("context-without-exclusive-owner");
        }
        for (unsigned j = 0; j < 8; j++)
        {
            if (canary[j] != UINT64_C(0xabd000) + slot * 16 + j)
            {
                sbg_fail("saved-stack-canary");
            }
        }
        if (slot == 0 && c != 0)
        {
            sbg_fail("pinned-task-moved");
        }
        if (SB_POLICY && slot == 1 && iteration == 2)
        {
            if (sb_affinity(&sbg_world, c, sbg_ids[1], 2, &sbg_stats[c]) !=
                    SB_DEFERRED ||
                sbg_world.mask[1] != 3 || sbg_world.pending[1] != 2)
            {
                sbg_fail("running-affinity-not-deferred");
            }
        }
        sbg_seen[slot] |= 1u << c;
        sbg_actions[slot]++;
        sbg_requested[slot] = iteration == 2
                                  ? SQ_BLOCKED
                                  : (iteration == 6 ? SQ_EXITED : SQ_READY);
        cmk2_switch(&sbg_sp[slot], sbg_idle[c]);
    }
    sbg_fail("exited-context-resumed");
}

static int sbg_step(unsigned c)
{
    struct sq_handle id;
    struct sq_event e;
    int result = sb_dispatch(&sbg_world, c, &id, &e, &sbg_stats[c]);
    if (result == SQ_IDLE)
    {
        return 0;
    }
    if (result != SQ_OK)
    {
        sbg_fail("dispatch-admission");
    }
    sbg_log(c, e);
    sbg_current[c] = id.slot;
    if (__atomic_exchange_n(&sbg_executing[id.slot], 1, __ATOMIC_ACQ_REL))
    {
        sbg_fail("duplicate-running-stack");
    }
    if (SB_GUEST_MUTANT == 1)
    {
        if (sb_finish(&sbg_world, c, id, SQ_READY, &e, &sbg_stats[c]) != SQ_OK)
        {
            sbg_fail("early-finish-control");
        }
    }
    if (atomic_load(&sbg_world.wake.queues.task[id.slot].state) != SQ_RUNNING)
    {
        sbg_fail("published-context-before-save");
    }
    uint64_t begin = sbg_ticks();
    cmk2_switch(&sbg_idle[c], sbg_sp[id.slot]);
    sbg_active[c] += sbg_ticks() - begin;
    sbg_switches[c] += 2;
    __atomic_store_n(&sbg_executing[id.slot], 0, __ATOMIC_RELEASE);
    if (sb_finish(&sbg_world, c, id, sbg_requested[id.slot], &e,
                  &sbg_stats[c]) != SQ_OK)
    {
        sbg_fail("saved-context-finish");
    }
    sbg_log(c, e);
    if (sbg_requested[id.slot] == SQ_BLOCKED)
    {
        __atomic_fetch_add(&sbg_blocked, 1, __ATOMIC_RELEASE);
    }
    if (sbg_requested[id.slot] == SQ_EXITED)
    {
        __atomic_fetch_add(&sbg_done, 1, __ATOMIC_RELEASE);
    }
    return 1;
}

static void sbg_run(unsigned c)
{
    uint64_t begin = sbg_ticks();
    if (c == 0)
    {
        while (__atomic_load_n(&sbg_blocked, __ATOMIC_ACQUIRE) < 4)
        {
            if (!sbg_step(c))
            {
                sbg_fail("initial-service-lost");
            }
        }
    }
    __atomic_store_n(&sbg_parked[c], 1, __ATOMIC_RELEASE);
    if (c == 0)
    {
        for (unsigned i = 1; i < topology.count; i++)
        {
            while (!__atomic_load_n(&sbg_parked[i], __ATOMIC_ACQUIRE))
            {
                pause_cpu();
            }
        }
        if (!sb_invariant(&sbg_world))
        {
            sbg_fail("blocked-snapshot");
        }
        struct sb_timer timer;
        if (sb_timer_arm(&sbg_world, 0, sbg_ids[1], &timer, &sbg_stats[0]) !=
            SQ_OK)
        {
            sbg_fail("timer-arm");
        }
        for (unsigned slot = 0; slot < 4; slot++)
        {
            struct sq_event e;
            if (SB_GUEST_MUTANT == 2 && slot == 3)
            {
                continue;
            }
            if (sb_wake(&sbg_world, 0, sbg_ids[slot], &e, &sbg_stats[0]) !=
                SQ_OK)
            {
                sbg_fail("remote-wake");
            }
            sbg_log(0, e);
        }
        if (sbg_world.wake.queues.queue[0].count != 4)
        {
            sbg_fail("lost-durable-wake");
        }
        if (SB_POLICY)
        {
            struct sq_transfer t;
            struct sq_event e;
            if (sb_move_begin(&sbg_world, 0, 1, sbg_ids[1], 0, &t, &e,
                              &sbg_stats[0]) != SQ_OK)
            {
                sbg_fail("saved-transfer-begin");
            }
            sbg_log(0, e);
            if (sb_timer_fire(&sbg_world, 0, timer, &e, &sbg_stats[0]) !=
                SW_MOVING)
            {
                sbg_fail("timer-not-retained-during-transfer");
            }
            if (sb_move_end(&sbg_world, 0, t, 1, &e, &sbg_stats[0]) != SQ_OK)
            {
                sbg_fail("saved-transfer-end");
            }
            sbg_log(0, e);
            if (sb_timer_fire(&sbg_world, 0, timer, &e, &sbg_stats[0]) !=
                SQ_DUPLICATE)
            {
                sbg_fail("timer-old-cpu-route");
            }
            struct sq_event first, last;
            if (sb_balance(&sbg_world, 0, &first, &last, &sbg_stats[0]) !=
                    SQ_OK ||
                first.slot != 2)
            {
                sbg_fail("balanced-unpinned-task");
            }
            sbg_log(0, first);
            sbg_log(0, last);
            if (sb_balance(&sbg_world, 0, &first, &last, &sbg_stats[0]) !=
                SQ_IDLE)
            {
                sbg_fail("unbounded-balancing");
            }
        }
        else
        {
            struct sq_event e;
            if (sb_timer_fire(&sbg_world, 0, timer, &e, &sbg_stats[0]) !=
                SQ_DUPLICATE)
            {
                sbg_fail("pinned-timer-route");
            }
        }
        if (!sb_invariant(&sbg_world))
        {
            sbg_fail("transfer-snapshot");
        }
        __atomic_store_n(&sbg_phase, 1, __ATOMIC_RELEASE);
    }
    else
    {
        while (!__atomic_load_n(&sbg_phase, __ATOMIC_ACQUIRE))
        {
            pause_cpu();
        }
    }
    while (__atomic_load_n(&sbg_done, __ATOMIC_ACQUIRE) < 4)
    {
        if (!sbg_step(c))
        {
            pause_cpu();
        }
    }
    sbg_elapsed[c] = sbg_ticks() - begin;
    __atomic_store_n(&sbg_finished[c], 1, __ATOMIC_RELEASE);
}

static void sbg_worker(unsigned c)
{
    while (!__atomic_load_n(&sbg_go, __ATOMIC_ACQUIRE))
    {
        pause_cpu();
    }
    sbg_run(c);
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

void sbg_bsp(void)
{
    if (topology.count < 2 ||
        sb_init(&sbg_world, topology.count, 4,
                (struct sw_port){sbg_ring, sbg_ack, NULL}) != SQ_OK)
    {
        sbg_fail("world-init");
    }
    for (unsigned slot = 0; slot < 4; slot++)
    {
        struct sq_event e;
        if (sb_create(&sbg_world, 0, 0, slot, SB_POLICY && slot ? 3 : 1,
                      &sbg_ids[slot], &e) != SQ_OK)
        {
            sbg_fail("task-create");
        }
        sbg_log(0, e);
        uint64_t *sp = (uint64_t *)(sbg_stack[slot] + 8192);
        *--sp = 0;
        *--sp = (uintptr_t)sbg_task;
        for (unsigned i = 0; i < 6; i++)
        {
            *--sp = 0;
        }
        sbg_sp[slot] = sp;
    }
    __atomic_store_n(&sbg_go, 1, __ATOMIC_RELEASE);
    sbg_run(0);
    for (unsigned c = 1; c < topology.count; c++)
    {
        while (!__atomic_load_n(&sbg_finished[c], __ATOMIC_ACQUIRE))
        {
            pause_cpu();
        }
    }
    if (!sb_invariant(&sbg_world))
    {
        sbg_fail("final-snapshot");
    }
    for (unsigned slot = 0; slot < 4; slot++)
    {
        unsigned expected = SB_POLICY && (slot == 1 || slot == 2) ? 3 : 1;
        if (sbg_actions[slot] != 7 || sbg_executing[slot] ||
            atomic_load(&sbg_world.wake.queues.task[slot].state) != SQ_EXITED ||
            sbg_seen[slot] != expected)
        {
            sbg_fail("task-completion-or-placement");
        }
        text("SB_TASK slot=");
        number(slot);
        text(" actions=");
        number(sbg_actions[slot]);
        text(" seen=");
        number(sbg_seen[slot]);
        text(" migrations=");
        ss_u64(sbg_world.migrations[slot]);
        text("\n");
    }
    for (unsigned c = 0; c < topology.count; c++)
    {
        text("SB_CPU actor=");
        number(c);
        text(" apic=");
        number(topology.ids[c]);
        text(" switches=");
        number(sbg_switches[c]);
        text(" active=");
        ss_u64(sbg_active[c]);
        text(" elapsed=");
        ss_u64(sbg_elapsed[c]);
        text("\n");
        for (unsigned seq = 0; seq < sbg_count[c]; seq++)
        {
            struct sq_event *e = &sbg_events[c][seq];
            text("SB_EVENT actor=");
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
    text("SB_GUEST_PASS:affinity-migration-timer policy=");
    number(SB_POLICY);
    text(" cpus=");
    number(topology.count);
    text(" tasks=4\n");
    unsigned value = 16;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
