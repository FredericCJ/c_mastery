#include "timer.h"
#ifndef PT_MUTANT
#define PT_MUTANT 0
#endif
static int valid(const struct pt_config *c)
{
    unsigned n;
    if (!c || !c->clock || c->clock > 180000000u || !c->generation ||
        c->psc > 65535u || !c->arr || c->center > 1u ||
        c->arr > (c->center ? 65535u : 65534u))
        return 0;
    n = c->arr + (c->center ? 0u : 1u);
    return c->ccr[0] <= n && c->ccr[1] <= n;
}

int pt_choose(const struct pu_clock *clock, unsigned psc, unsigned hz,
              unsigned d0, unsigned d1, unsigned center, unsigned ppm,
              struct pt_config *out)
{
    struct pu_rates rates;
    struct pt_config c;
    uint64_t base, actual, error;
    uint32_t b, n, rem;
    if (!clock || !out || psc > 65535u || !hz || d0 > 1000u || d1 > 1000u ||
        center > 1u || ppm > 1000000u || pu_rates(clock, &rates) != PU_OK)
        return PT_INVALID;
    base = (uint64_t)(psc + 1u) * hz * (center ? 2u : 1u);
    if (base > rates.timer1)
        return PT_RANGE;
    b = (uint32_t)base;
    n = rates.timer1 / b;
    rem = rates.timer1 % b;
    if (rem >= (b / 2u + b % 2u))
        ++n; /* nearest period; ties upward */
    if (n < (center ? 1u : 2u) || n > 65535u)
        return PT_RANGE;
    actual = base * n;
    error =
        actual > rates.timer1 ? actual - rates.timer1 : rates.timer1 - actual;
    if (error * 1000000u > actual * ppm)
        return PT_RANGE;
    c.clock = rates.timer1;
    c.generation = clock->generation;
    c.psc = psc;
    c.arr = n - (center ? 0u : 1u);
    c.ccr[0] = (n * d0 + 500u) / 1000u;
    c.ccr[1] = (n * d1 + 500u) / 1000u;
    c.center = center;
    *out = c;
    return PT_OK;
}

int pt_ratios(const struct pt_config *c, struct pt_ratio *f,
              struct pt_ratio *tick, struct pt_ratio d[2])
{
    uint32_t n;
    if (!valid(c) || !f || !tick || !d)
        return PT_INVALID;
    n = c->center ? 2u * c->arr : c->arr + 1u;
    if (PT_MUTANT == 1)
        n = c->arr;
    if (PT_MUTANT == 2 && c->center)
        n = c->arr + 1u;
    f->numerator = c->clock;
    f->denominator = (uint64_t)(c->psc + 1u) * n;
    tick->numerator = c->psc + 1u;
    tick->denominator = c->clock;
    d[0].numerator = c->ccr[0];
    d[1].numerator = c->ccr[1];
    d[0].denominator = d[1].denominator = c->arr + (c->center ? 0u : 1u);
    return PT_OK;
}

int pt_init(struct pt_model *m, const struct pt_config *c)
{
    if (!m || !valid(c))
        return PT_INVALID;
    m->active = *c;
    m->shadow = *c;
    m->counter = 0;
    m->down = 0;
    m->prescale = 0;
    m->udis = 0;
    m->urs = 0;
    m->uif = 0;
    m->running = 1;
    m->updates = 0;
    return PT_OK;
}

int pt_stage(struct pt_model *m, const struct pt_config *c)
{
    if (!m || !valid(c))
        return PT_INVALID;
    if (c->generation != m->active.generation || c->clock != m->active.clock)
        return PT_STALE;
    if (c->center != m->active.center)
        return PT_BUSY;
    m->shadow = *c;
    if (PT_MUTANT == 3)
        m->active = *c;
    return PT_OK;
}

static void update(struct pt_model *m, unsigned software)
{
    if (m->udis)
        return;
    if (PT_MUTANT == 4)
    {
        unsigned old = m->active.ccr[1];
        m->active = m->shadow;
        m->active.ccr[1] = old;
    }
    else
        m->active = m->shadow;
    if (m->updates != UINT32_MAX)
        ++m->updates;
    if (!software || !m->urs)
        m->uif = 1;
}

int pt_ug(struct pt_model *m)
{
    if (!m)
        return PT_INVALID;
    update(m, 1);
    m->counter = 0;
    m->down = 0;
    m->prescale = 0;
    return PT_OK;
}

int pt_tick(struct pt_model *m)
{
    if (!m)
        return PT_INVALID;
    if (!m->running)
        return PT_BUSY;
    if (!m->active.center)
    {
        if (m->counter >= m->active.arr)
        {
            update(m, 0);
            m->counter = 0;
        }
        else
            ++m->counter;
    }
    else if (!m->down)
    {
        if (m->counter + 1u >= m->active.arr)
        {
            update(m, 0);
            m->counter = m->active.arr;
            m->down = 1;
        }
        else
            ++m->counter;
    }
    else if (m->counter <= 1u)
    {
        update(m, 0);
        m->counter = 0;
        m->down = 0;
    }
    else
        --m->counter;
    return PT_OK;
}

int pt_input(struct pt_model *m)
{
    if (!m)
        return PT_INVALID;
    if (!m->running)
        return PT_BUSY;
    if (m->prescale == m->active.psc)
    {
        m->prescale = 0;
        return pt_tick(m);
    }
    ++m->prescale;
    return PT_OK;
}

int pt_level(const struct pt_model *m, unsigned channel, unsigned *out)
{
    unsigned c;
    if (!m || !out || channel > 1u)
        return PT_INVALID;
    c = m->active.ccr[channel];
    /* Explicit extremes agree with RM0090 Figure165, including CCR=ARR. */
    if (!c)
        *out = 0;
    else if (m->active.center && c >= m->active.arr)
        *out = 1;
    else
        *out = m->active.center && m->down ? m->counter <= c : m->counter < c;
    return PT_OK;
}

int pt_interval(unsigned previous, unsigned current, unsigned max_gap,
                unsigned overcapture, unsigned *ticks)
{
    unsigned d;
    if (!ticks || previous > 65535u || current > 65535u || !max_gap ||
        max_gap >= 32768u || overcapture > 1u)
        return PT_INVALID;
    if (overcapture && PT_MUTANT != 5)
        return PT_LOST;
    d = (current - previous) & 65535u;
    if (!d || (d > max_gap && PT_MUTANT != 6))
        return PT_RANGE;
    *ticks = d;
    return PT_OK;
}
