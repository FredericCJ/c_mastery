#include "transceivers.h"
#include <limits.h>
#ifndef TR_MUTANT
#define TR_MUTANT 0
#endif

int tr_sample(const struct tr_sample *s, int *out)
{
    int decision;
    int64_t differential, common_twice;
    if (!s || !out || s->device > TR_AM26LV32E || s->enabled > 1 ||
        s->open > 1 || s->swapped > 1)
    {
        return TR_INVALID;
    }
    if (s->device <= TR_MAX3232_33)
    {
        if (s->swapped || s->b_mv || s->a_mv < -25000 || s->a_mv > 25000)
        {
            return TR_INVALID;
        }
        if (!s->enabled)
        {
            return TR_INVALID; /* no enable pin on this part */
        }
        if (s->open)
        {
            decision = s->device == TR_MAX3232_33 ? TR_HIGH : TR_UNKNOWN;
        }
        else if (s->device == TR_RS232_FIXTURE)
        {
            decision = s->a_mv <= -3000  ? TR_HIGH
                       : s->a_mv >= 3000 ? TR_LOW
                                         : TR_UNKNOWN;
        }
        else
        {
            decision = s->a_mv <= 600    ? TR_HIGH
                       : s->a_mv >= 2400 ? TR_LOW
                                         : TR_UNKNOWN;
        }
        if (TR_MUTANT == 1 && s->device == TR_MAX3232_33)
        {
            decision =
                s->a_mv < 1500 ? TR_HIGH : TR_LOW; /* typical threshold */
        }
    }
    else
    {
        differential = (int64_t)s->a_mv - s->b_mv;
        common_twice = (int64_t)s->a_mv + s->b_mv;
        if (differential < -7000 || differential > 7000 ||
            common_twice < -14000 || common_twice > 14000)
        {
            return TR_INVALID;
        }
        if (!s->enabled)
        {
            decision = TR_HIGH_Z;
        }
        else if (s->open)
        {
            decision = s->device == TR_AM26LV32E ? TR_HIGH : TR_UNKNOWN;
        }
        else
        {
            if (s->swapped)
            {
                differential = -differential;
            }
            decision = differential >= 200    ? TR_HIGH
                       : differential <= -200 ? TR_LOW
                                              : TR_UNKNOWN;
            if (TR_MUTANT == 2 && differential == 0)
            {
                decision = TR_HIGH;
            }
        }
    }
    *out = decision;
    return TR_OK;
}

static int connector(const struct tr_connector *p)
{
    return p && p->role <= TR_DCE && p->td >= 1 && p->td <= 3 && p->rd >= 1 &&
           p->rd <= 3 && p->reference >= 1 && p->reference <= 3 &&
           p->td != p->rd && p->td != p->reference && p->rd != p->reference;
}

static int direction(const struct tr_connector *p, unsigned pin)
{
    if (pin == p->reference)
    {
        return 2;
    }
    return pin == p->td ? (p->role == TR_DTE ? 1 : 0)
                        : (p->role == TR_DCE ? 1 : 0);
}

int tr_connections(const struct tr_connector *a, const struct tr_connector *b,
                   const struct tr_wire *wires, size_t count)
{
    unsigned left = 0, right = 0;
    if (!connector(a) || !connector(b) || !wires || count != 3)
    {
        return TR_INVALID;
    }
    for (size_t i = 0; i < count; ++i)
    {
        unsigned x = wires[i].left, y = wires[i].right;
        int dx, dy;
        if (x < 1 || x > 3 || y < 1 || y > 3 || (left & (1u << x)) ||
            (right & (1u << y)))
        {
            return TR_INVALID;
        }
        left |= 1u << x;
        right |= 1u << y;
        dx = direction(a, x);
        dy = direction(b, y);
        if (TR_MUTANT == 3)
        {
            continue;
        }
        if ((dx == 2 || dy == 2) ? dx != dy : dx == dy)
        {
            return TR_INVALID;
        }
    }
    return TR_OK;
}

int tr_topology(unsigned drivers, unsigned receivers, unsigned termination,
                unsigned reference)
{
    if (drivers != 1 || !receivers || receivers > 10 || termination != 1 ||
        reference != 1)
    {
        return TR_INVALID;
    }
    return TR_OK;
}

int tr_232_circuit(const struct tr232_circuit *c)
{
    if (!c || c->supply_mv < 3000 || c->supply_mv > 3600 || !c->baud ||
        c->baud > 115200 || c->resistance_ohm != 3000 ||
        c->capacitance_pf != 1000 || c->temperature_c < -40 ||
        c->temperature_c > 85 || c->bypass_nf != 100)
    {
        return TR_INVALID;
    }
    for (size_t i = 0; i < 4; ++i)
    {
        if (c->pump_nf[i] != 100)
        {
            return TR_INVALID;
        }
    }
    return TR_OK;
}

int tr_capture(unsigned device, const struct tr_point *p, size_t count,
               unsigned swapped, struct tr_result *out)
{
    struct tr_result r = {0, 0, 0, SIZE_MAX};
    unsigned required = TR_HAVE_LOGIC | TR_HAVE_LINE | TR_HAVE_RETURN;
    if (!out || !p || !count || count > 160 || device > TR_AM26LV32E ||
        swapped > 1)
    {
        return TR_INVALID;
    }
    if (device >= TR_RS422_FIXTURE)
    {
        required |= TR_HAVE_PAIR;
    }
    for (size_t i = 0; i < count; ++i)
    {
        int d;
        struct tr_sample sample = {device, 1, 0, swapped, p[i].a_mv, p[i].b_mv};
        if (p[i].expected > 1 || p[i].returned > 1 || p[i].columns > 15 ||
            (i && p[i].time_ns <= p[i - 1].time_ns))
        {
            return TR_INVALID;
        }
        if (TR_MUTANT != 4 && (p[i].columns & required) != required)
        {
            return TR_INCOMPLETE;
        }
        if (tr_sample(&sample, &d) != TR_OK)
        {
            return TR_INVALID;
        }
        if (d == TR_UNKNOWN)
        {
            ++r.unknown;
            if (r.first_bad == SIZE_MAX)
            {
                r.first_bad = i;
            }
        }
        else if (d != (int)p[i].expected || p[i].returned != p[i].expected)
        {
            ++r.mismatched;
            if (r.first_bad == SIZE_MAX)
            {
                r.first_bad = i;
            }
        }
        else
        {
            ++r.matched;
        }
    }
    *out = r;
    return r.unknown || r.mismatched ? TR_MISMATCH : TR_OK;
}

int tr_edge(const struct tr_point *p, size_t count, unsigned expected,
            unsigned swapped, struct tr_edge *out)
{
    struct tr_edge r = {0, 0, 0, 0};
    int previous = TR_UNKNOWN;
    if (!out || !p || count < 2 || count > 32 || expected > 1 || swapped > 1)
    {
        return TR_INVALID;
    }
    for (size_t i = 0; i < count; ++i)
    {
        int d;
        struct tr_sample s = {TR_AM26LV32E, 1,         0,
                              swapped,      p[i].a_mv, p[i].b_mv};
        if ((i && p[i].time_ns <= p[i - 1].time_ns) || p[i].columns != 15)
        {
            return TR_INVALID;
        }
        if (tr_sample(&s, &d) != TR_OK)
        {
            ++r.outside;
            previous = TR_UNKNOWN;
            continue;
        }
        if (d == TR_UNKNOWN)
        {
            ++r.center;
            continue;
        }
        if (d != (int)expected)
        {
            ++r.opposite;
        }
        if (TR_MUTANT != 6 && previous != TR_UNKNOWN && d != previous)
        {
            ++r.transitions;
        }
        previous = d;
    }
    *out = r;
    return TR_OK;
}

int tr_grant(struct tr_bus *b, unsigned peer, uint32_t serial, uint64_t now)
{
    if (!b || peer > 2 || b->phase > 2 || now < b->last)
    {
        return TR_INVALID;
    }
    if (b->phase && TR_MUTANT != 5)
    {
        return TR_BUSY;
    }
    if (!serial || serial <= b->serial)
    {
        return TR_STALE;
    }
    b->serial = serial;
    b->owner = peer;
    b->phase = 1;
    b->last = now;
    return TR_OK;
}

int tr_release(struct tr_bus *b, unsigned peer, uint32_t serial, uint64_t now,
               unsigned tc, uint64_t disable, uint64_t guard)
{
    uint64_t delay;
    if (!b || peer > 2 || tc > 1 || now < b->last || b->phase > 2)
    {
        return TR_INVALID;
    }
    if (b->phase != 1 || peer != b->owner || serial != b->serial)
    {
        return TR_STALE;
    }
    if (!tc)
    {
        return TR_BUSY;
    }
    if (disable > UINT64_MAX - guard)
    {
        return TR_INVALID;
    }
    delay = disable + guard;
    if (now > UINT64_MAX - delay)
    {
        return TR_INVALID;
    }
    b->last = now;
    b->ready_at = now + delay;
    b->phase = 2;
    return TR_OK;
}

int tr_ready(struct tr_bus *b, uint64_t now)
{
    if (!b || b->phase > 2 || now < b->last)
    {
        return TR_INVALID;
    }
    if (b->phase != 2 || now < b->ready_at)
    {
        return TR_BUSY;
    }
    b->last = now;
    b->phase = 0;
    return TR_OK;
}
