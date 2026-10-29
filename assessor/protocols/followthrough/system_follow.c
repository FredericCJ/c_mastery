#include "system_follow.h"
#ifndef PS_MUTANT
#define PS_MUTANT 0
#endif
int pf_system_init(struct pf_system *s, struct fw_io io, uint32_t epoch,
                   pf_free_fn free_fn, void *ctx)
{
    if (!s || !io.read || !io.write || !epoch || !free_fn)
    {
        return PT_INVALID;
    }
    *s = (struct pf_system){0};
    cmx_init(&s->kernel, CMX_FIXED);
    for (unsigned i = 0; i < 3; ++i)
    {
        if (cmx_task_add(&s->kernel, 3 - i, 100, (uintptr_t)s->stacks[i],
                         sizeof s->stacks[i]) != (int)i)
        {
            return PT_INVALID;
        }
    }
    cmx_queue_init(&s->queue, 4);
    ad_pool_init(&s->pool);
    if (algo_ring_init(&s->history, 16) != ALGO_OK)
    {
        return PT_INVALID;
    }
    if (fw_gpio(io, 7, 1) || fw_timer(io, 1000000, 1000, 500) ||
        fw_uart(io, 0xA5, 3))
    {
        return PT_RECOVERY;
    }
    return pf_driver_init(&s->driver, io, epoch, free_fn, ctx);
}

int pf_system_post(struct pf_system *s, unsigned producer, unsigned kind,
                   struct pf_recovery r, uint32_t epoch, unsigned idle,
                   struct ad_brand *out)
{
    if (!s || !out || producer > 1 || kind < PF_COMPLETE || kind > PF_QUIET ||
        idle > 1 || r.owner != &s->driver)
    {
        return PT_INVALID;
    }
    struct ad_brand brand;
    if (!ad_pool_acquire(&s->pool, kind, &brand))
    {
        return PT_RANGE;
    }
    s->messages[brand.index] = (struct pf_message){brand, r, epoch, kind, idle};
    enum cmx_result rc = cmx_queue_send(&s->kernel, &s->queue, producer,
                                        brand.index, cmx_at(s->kernel.now));
    if (rc != CMX_OK)
    {
        (void)ad_pool_release(&s->pool, brand);
        return PT_RANGE;
    }
    *out = brand;
    return PT_OK;
}

int pf_system_step(struct pf_system *s)
{
    if (!s)
    {
        return PT_INVALID;
    }
    enum cmx_result q =
        cmx_queue_recv(&s->kernel, &s->queue, 2, cmx_at(s->kernel.now));
    if (q == CMX_TIMEOUT)
    {
        return PT_PENDING;
    }
    if (q != CMX_OK)
    {
        return PT_STATE;
    }
    unsigned i = s->kernel.task[2].value;
    if (i >= 4)
    {
        return PT_INVALID;
    }
    struct pf_message *m = &s->messages[i];
    uint32_t kind;
    if (!ad_pool_get(&s->pool, m->brand, &kind) || kind != m->kind)
    {
        return PT_STATE;
    }
    int r;
    if (m->kind == PF_RESET)
    {
        r = pf_driver_reset(&m->recovery);
    }
    else if (m->kind == PF_COMPLETE)
    {
        r = pf_driver_complete(&m->recovery, m->epoch,
                               PS_MUTANT == 1 ? 1 : m->idle);
    }
    else
    {
        r = pf_driver_recover(&m->recovery, m->epoch, m->idle);
    }
    if (PS_MUTANT != 2)
    {
        (void)ad_pool_release(&s->pool, m->brand);
    }
    if (algo_ring_put(&s->history, r) != ALGO_OK)
    {
        ++s->lost_history;
    }
    return r;
}

int pf_system_arm_deadline(struct pf_system *s, struct pf_recovery r,
                           uint64_t now, unsigned timeout)
{
    if (!s || r.owner != &s->driver || !timeout || timeout > 1000)
    {
        return PT_INVALID;
    }
    if (s->timer_armed || !s->driver.owned || r.epoch != s->driver.loan_epoch ||
        r.ticket != s->driver.dma.generation)
    {
        return PT_STATE;
    }
    if (now < s->last_time || now > UINT64_MAX - timeout)
    {
        return PT_RANGE;
    }
    s->deadline = now + timeout;
    s->last_time = now;
    s->timer_recovery = r;
    s->timer_armed = 1;
    return PT_OK;
}

int pf_system_tick(struct pf_system *s, uint64_t now)
{
    if (!s)
    {
        return PT_INVALID;
    }
    if (now < s->last_time)
    {
        return PT_RANGE;
    }
    if (!s->timer_armed)
    {
        s->last_time = now;
        return PT_OK;
    }
    if (now < s->deadline)
    {
        s->last_time = now;
        return PT_PENDING;
    }
    struct ad_brand brand;
    int r = pf_system_post(s, 1, PF_RESET, s->timer_recovery,
                           s->timer_recovery.epoch, 0, &brand);
    if (r)
    {
        return r; /* Full queue retains the armed timeout for an explicit retry. */
    }
    s->last_time = now;
    s->timer_armed = 0;
    return PT_OK;
}
