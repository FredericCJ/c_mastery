#include "supervision.h"
#include <limits.h>
#ifndef CS_MUTANT
#define CS_MUTANT 0
#endif
_Static_assert(UINT_MAX == UINT32_MAX,
               "selected32-bit unsigned atomic representation");

enum cs_status cs_init(struct cs_world *w, unsigned n, unsigned b)
{
    if (!w || !n || n > CS_TASKS || !b)
    {
        return CS_INVALID;
    }
    atomic_init(&w->sequence, 0);
    for (unsigned i = 0; i < CS_TASKS; ++i)
    {
        atomic_init(&w->epoch[i], 0);
        w->last[i] = 0;
    }
    w->count = n;
    w->boot = b;
    return CS_OK;
}

enum cs_status cs_progress(struct cs_world *w, unsigned t)
{
    if (!w || !w->count || w->count > CS_TASKS || t >= w->count)
    {
        return CS_INVALID;
    }
    unsigned old = atomic_load(&w->sequence);
    if (old & 1)
    {
        return CS_BUSY;
    }
    if (old > UINT_MAX - 2)
    {
        return CS_EXHAUSTED;
    }
    if (!atomic_compare_exchange_strong(&w->sequence, &old, old + 1))
    {
        return CS_BUSY;
    }
    unsigned epoch = atomic_load(&w->epoch[t]);
    if (epoch == UINT_MAX)
    {
        atomic_store(&w->sequence, old + 2);
        return CS_EXHAUSTED;
    }
    atomic_store(&w->epoch[t], epoch + 1);
    atomic_store(&w->sequence, old + 2);
    return CS_OK;
}

enum cs_status cs_window(struct cs_world *w)
{
    if (!w || !w->count || w->count > CS_TASKS || !w->boot)
    {
        return CS_INVALID;
    }
    unsigned start = atomic_load(&w->sequence), copy[CS_TASKS];
    if (start & 1)
    {
        return CS_BUSY;
    }
    for (unsigned i = 0; i < w->count; ++i)
    {
        copy[i] = atomic_load(&w->epoch[i]);
    }
#ifdef CS_TEST_HOOK
    cs_snapshot_hook(w);
#endif
    if (start != atomic_load(&w->sequence))
    {
        return CS_BUSY;
    }
    unsigned advanced = 0;
    for (unsigned i = 0; i < w->count; ++i)
    {
        if (copy[i] < w->last[i])
        {
            return CS_INVALID;
        }
        if (copy[i] > w->last[i])
        {
            ++advanced;
        }
    }
    if (advanced != (CS_MUTANT == 1 ? 1 : w->count))
    {
        if (CS_MUTANT == 2)
        {
            for (unsigned i = 0; i < w->count; ++i)
            {
                w->last[i] = copy[i];
            }
        }
        return CS_WAIT;
    }
    for (unsigned i = 0; i < w->count; ++i)
    {
        w->last[i] = copy[i];
    }
    return CS_OK;
}

enum cs_status cs_bitmap_init(struct cs_bitmap *w, unsigned n)
{
    if (!w || !n || n > CS_TASKS)
    {
        return CS_INVALID;
    }
    atomic_init(&w->pending, 0);
    w->required = (1u << n) - 1;
    w->seen = 0;
    return CS_OK;
}

enum cs_status cs_bitmap_progress(struct cs_bitmap *w, unsigned t)
{
    if (!w || t >= CS_TASKS || !(w->required & (1u << t)))
    {
        return CS_INVALID;
    }
    atomic_fetch_or(&w->pending, 1u << t);
    return CS_OK;
}

enum cs_status cs_bitmap_window(struct cs_bitmap *w)
{
    if (!w || !w->required || w->required > 15)
    {
        return CS_INVALID;
    }
    unsigned observed = CS_MUTANT == 6 ? atomic_load(&w->pending)
                                       : atomic_exchange(&w->pending, 0);
#ifdef CS_TEST_HOOK
    cs_bitmap_hook(w);
#endif
    if (CS_MUTANT == 6)
    {
        atomic_store(&w->pending, 0);
    }
    w->seen |= observed;
    if (CS_MUTANT == 3)
    {
        w->seen = observed;
    }
    if (w->seen != w->required)
    {
        return CS_WAIT;
    }
    w->seen = 0;
    return CS_OK;
}

static void seal(struct cs_recovery *r)
{
    r->inverse[0] = ~r->magic;
    r->inverse[1] = ~r->attempts;
    r->inverse[2] = ~r->reason;
    r->inverse[3] = ~r->terminal;
}

enum cs_status cs_recovery_init(struct cs_recovery *r)
{
    if (!r)
    {
        return CS_INVALID;
    }
    *r = (struct cs_recovery){.magic = UINT32_C(0x43535231)};
    seal(r);
    return CS_OK;
}

enum cs_status cs_failure(struct cs_recovery *r, enum cs_reason why)
{
    if (!r || why < CS_HUNG || why > CS_BOOT_FAILED)
    {
        return CS_INVALID;
    }
    unsigned valid = r->magic == UINT32_C(0x43535231) && r->attempts <= 3 &&
                     r->terminal <= 1 && r->reason <= CS_CORRUPT;
    if (CS_MUTANT != 4)
    {
        valid = valid && r->inverse[0] == ~r->magic &&
                r->inverse[1] == ~r->attempts && r->inverse[2] == ~r->reason &&
                r->inverse[3] == ~r->terminal;
    }
    if (!valid)
    {
        *r = (struct cs_recovery){.magic = UINT32_C(0x43535231),
                                  .attempts = 3,
                                  .reason = CS_CORRUPT,
                                  .terminal = 1};
        seal(r);
        return CS_SAFE;
    }
    if (r->terminal)
    {
        return CS_SAFE;
    }
    r->reason = (uint32_t)why;
    if (r->attempts == 3)
    {
        r->terminal = 1;
        seal(r);
        return CS_SAFE;
    }
    if (CS_MUTANT != 5)
    {
        ++r->attempts;
    }
    seal(r);
    return CS_RETRY;
}

static unsigned product(uint64_t a, uint64_t b, uint64_t *out)
{
    if (b && a > UINT64_MAX / b)
    {
        return 0;
    }
    *out = a * b;
    return 1;
}

enum cs_status cs_energy(const struct cs_power *p, unsigned n, uint64_t *out)
{
    if (!out || n > 16 || (n && !p))
    {
        return CS_INVALID;
    }
    uint64_t total = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        uint64_t power, energy;
        if (!product(p[i].millivolts, p[i].microamps, &power) ||
            !product(power, p[i].microseconds, &energy) ||
            energy > UINT64_MAX - total)
        {
            return CS_EXHAUSTED;
        }
        total += energy;
    }
    *out = total;
    return CS_OK;
}
