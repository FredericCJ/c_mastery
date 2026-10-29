#include "breaker.h"
#include <limits.h>

int rzb_init(struct rzb_gate *g, unsigned a, unsigned b,
             struct rzb_policy policy, uint64_t now)
{
    if (!g || !a || a > 4 || !b || b > 4 || !policy.window ||
        policy.window > 8 || !policy.minimum ||
        policy.minimum > policy.window || !policy.numerator ||
        policy.numerator > policy.denominator || policy.denominator > 16)
    {
        return RZ_INVALID;
    }
    *g = (struct rzb_gate){.policy = policy,
                           .reserved = {a, b},
                           .last = now,
                           .health = {{.epoch = 1}, {.epoch = 1}}};
    return RZ_OK;
}

static bool valid(const struct rzb_gate *g, struct rzb_permit p)
{
    return p.owner == g && p.slot < 4 && g->slots[p.slot].active &&
           g->slots[p.slot].generation == p.generation;
}

int rzb_admit(struct rzb_gate *g, unsigned tenant, unsigned cost,
              struct rzf_scope *scope, uint64_t now, struct rzb_permit *out)
{
    if (!g || !scope || !out || tenant > 1 || !cost ||
        cost > g->reserved[tenant])
    {
        return RZ_INVALID;
    }
    if (now < g->last)
    {
        return RZ_RANGE;
    }
    if (g->closed)
    {
        return RZ_CLOSED;
    }
    int status = rz_check(&scope->core, now);
    if (status != RZ_OK)
    {
        return status;
    }
    struct rzb_health *health = &g->health[tenant];
    if (health->retired || (health->phase == 1 && now < health->until))
    {
        return RZ_OPEN;
    }
    if (cost > g->reserved[tenant] - g->used[tenant])
    {
        return RZ_BUSY;
    }
    unsigned phase = health->phase == 1 ? 2 : health->phase;
    if (phase == 2)
    {
        for (unsigned i = 2 * tenant; i < 2 * tenant + 2; ++i)
        {
            if (g->slots[i].active && g->slots[i].epoch == health->epoch)
            {
                return RZ_BUSY;
            }
        }
    }
    unsigned index = 2 * tenant;
    while (index < 2 * tenant + 2 &&
           (g->slots[index].active || g->slots[index].generation == UINT_MAX))
    {
        ++index;
    }
    if (index == 2 * tenant + 2)
    {
        return RZ_FULL;
    }
    struct rzf_borrow borrow = {0};
    status = rzf_borrow_begin(&borrow, scope);
    if (status != RZ_OK)
    {
        return status;
    }
    struct rzb_slot *slot = &g->slots[index];
    ++slot->generation;
    slot->epoch = health->epoch;
    slot->cost = cost;
    slot->active = true;
    slot->cancelled = false;
    slot->scope = borrow;
    g->used[tenant] += cost;
    g->last = now;
    health->phase = phase;
    *out = (struct rzb_permit){g, index, slot->generation};
    return RZ_OK;
}

static void trip(struct rzb_health *h, uint64_t now)
{
    h->phase = 1;
    if (h->epoch == UINT_MAX || now > UINT64_MAX - 4)
    {
        h->retired = true;
        return;
    }
    ++h->epoch;
    h->until = now + 4;
}

int rzb_complete(struct rzb_gate *g, struct rzb_permit p, uint64_t now,
                 int outcome)
{
    if (!g || outcome < -1 || outcome > 1)
    {
        return RZ_INVALID;
    }
    if (now < g->last)
    {
        return RZ_RANGE;
    }
    if (!valid(g, p))
    {
        return RZ_STALE;
    }
    struct rzb_slot *slot = &g->slots[p.slot];
    unsigned tenant = p.slot / 2, epoch = slot->epoch;
    g->used[tenant] -= slot->cost;
    slot->active = false;
    g->last = now;
    (void)rzf_borrow_ack(&slot->scope);
    struct rzb_health *h = &g->health[tenant];
#if RZF_MUTANT != 61
    if (epoch != h->epoch)
    {
        return RZ_OK;
    }
#else
    (void)epoch;
#endif
    if (outcome < 0 || h->retired)
    {
        return RZ_OK;
    }
    if (h->phase == 2)
    {
        if (outcome)
        {
            h->phase = 0;
            h->count = h->head = h->failures = 0;
            for (unsigned i = 0; i < 8; ++i)
            {
                h->samples[i] = false;
            }
        }
        else
        {
            trip(h, now);
        }
    }
    else
    {
        if (h->count == g->policy.window)
        {
#if RZF_MUTANT != 62
            h->failures -= h->samples[h->head];
#endif
        }
        else
        {
            ++h->count;
        }
        h->samples[h->head] = outcome == 0;
        h->failures += outcome == 0;
        h->head = (h->head + 1) % g->policy.window;
        if (h->count >= g->policy.minimum &&
            h->failures * g->policy.denominator >=
                h->count * g->policy.numerator)
        {
            trip(h, now);
        }
    }
    return RZ_OK;
}

int rzb_cancel(struct rzb_gate *g, struct rzb_permit p)
{
    if (!g)
    {
        return RZ_INVALID;
    }
    if (!valid(g, p))
    {
        return RZ_STALE;
    }
    g->slots[p.slot].cancelled = true;
#if RZF_MUTANT == 63
    g->used[p.slot / 2] -= g->slots[p.slot].cost;
#endif
    return RZ_OK;
}

int rzb_cancelled(struct rzb_gate *g, struct rzb_permit p, uint64_t now,
                  bool *out)
{
    if (!g || !out)
    {
        return RZ_INVALID;
    }
    if (now < g->last)
    {
        return RZ_RANGE;
    }
    if (!valid(g, p))
    {
        return RZ_STALE;
    }
    *out = g->slots[p.slot].cancelled ||
           rz_check(&g->slots[p.slot].scope.scope->core, now) != RZ_OK;
    return RZ_OK;
}

int rzb_close(struct rzb_gate *g)
{
    if (!g)
    {
        return RZ_INVALID;
    }
    g->closed = true;
    for (unsigned i = 0; i < 4; ++i)
    {
        if (g->slots[i].active)
        {
            g->slots[i].cancelled = true;
        }
    }
    return RZ_OK;
}

int rzb_quiescent(const struct rzb_gate *g)
{
    if (!g)
    {
        return RZ_INVALID;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if (g->slots[i].active)
        {
            return RZ_BUSY;
        }
    }
    return RZ_OK;
}
