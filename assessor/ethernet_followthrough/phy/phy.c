#include "phy.h"
#ifndef EP_MUTANT
#define EP_MUTANT 0
#endif
static int bus_ok(const struct ed_bus *b)
{
    return b && b->read && b->write && b->clock;
}

int ep_write(const struct ed_bus *b, uint32_t hz, unsigned address,
             unsigned reg, unsigned polls, uint32_t timeout, uint16_t value)
{
    uint32_t word, div, status, start, last, now;
    int e;
    if (!bus_ok(b) || !polls || polls > 1024 || !timeout)
        return ED_INVALID;
    e = ed_mdio_word(hz, address, reg, 0, &word, &div);
    if (e)
        return e;
    (void)div;
    if (b->read(b->context, 0, &status))
        return ED_IO;
    if ((status & 1u) && EP_MUTANT != 4)
        return ED_BUSY;
    start = last = b->clock(b->context);
    if (b->write(b->context, 0, word & ~1u) || b->write(b->context, 1, value) ||
        b->write(b->context, 0, word | 2u))
        return ED_IO;
    for (unsigned i = 0; i < polls + (EP_MUTANT == 5); i++)
    {
        now = b->clock(b->context);
        if (now < last || now - start >= timeout)
            return ED_TIMEOUT;
        last = now;
        if (b->read(b->context, 0, &status))
            return ED_IO;
        if (!(status & 1u))
            return ED_OK;
    }
    return ED_TIMEOUT;
}

int ep_find(const struct ed_bus *b, uint32_t hz, unsigned polls,
            uint32_t timeout, unsigned *address)
{
    unsigned found = 32, matches = 0;
    uint16_t first, second;
    int e;
    if (!address || !bus_ok(b) || !polls || polls > 1024 || !timeout)
        return ED_INVALID;
    for (unsigned i = 0; i < 32; i++)
    {
        e = ed_mdio_read(b, hz, i, 2, polls, timeout, &first);
        if (e)
            return e;
        e = ed_mdio_read(b, hz, i, 3, polls, timeout, &second);
        if (e)
            return e;
        if (first == 7 && (second & 0xfff0u) == 0xc130u)
        {
            found = i;
            matches++;
        }
    }
    if (!matches)
        return ED_WRONG_PHY;
    if (matches != 1)
        return ED_INVALID;
    *address = found;
    return ED_OK;
}

int ep_init(struct ep_phy *s, const struct ed_bus *b, uint32_t hz,
            unsigned address, unsigned polls, unsigned timeout)
{
    uint32_t word, div;
    int e;
    if (!s || !bus_ok(b) || !polls || polls > 1024 || !timeout)
        return ED_INVALID;
    e = ed_mdio_word(hz, address, 0, 0, &word, &div);
    if (e)
        return e;
    if (s->initialized)
        return ED_BUSY;
    struct ep_phy next = {0};
    next.bus = *b;
    next.hclk = hz;
    next.address = address;
    next.access_polls = polls;
    next.access_timeout = timeout;
    next.initialized = 1;
    *s = next;
    return ED_OK;
}

static int retire(struct ep_phy *s)
{
    s->ready = 0;
    s->phase = EP_FAILED;
    if (s->incarnation == UINT32_MAX)
        return ED_FULL;
    if (EP_MUTANT != 1)
        s->incarnation++;
    return ED_OK;
}

int ep_invalidate(struct ep_phy *s)
{
    if (!s || !s->initialized)
        return ED_INVALID;
    return retire(s);
}

static int failed(struct ep_phy *s, int result)
{
    s->ready = 0;
    s->phase = EP_FAILED;
    return result;
}

int ep_begin(struct ep_phy *s, unsigned steps, uint32_t timeout)
{
    if (!s || !s->initialized || !steps || steps > 1024 || !timeout)
        return ED_INVALID;
    if (s->phase == EP_RESET || s->phase == EP_RESTART ||
        s->phase == EP_RESTART_WAIT || s->phase == EP_NEGOTIATE)
        return ED_BUSY;
    int e = retire(s);
    if (e)
        return e;
    s->start = s->last = s->bus.clock(s->bus.context);
    s->deadline = timeout;
    s->remaining = steps;
    /* PHY reset alone: do not combine reset with restart/enable bits. */
    e = ep_write(&s->bus, s->hclk, s->address, 0, s->access_polls,
                 s->access_timeout, EP_MUTANT == 2 ? 0x9200u : 0x8000u);
    if (e)
        return failed(s, e);
    s->phase = EP_RESET;
    return ED_BUSY;
}

static int time_ok(struct ep_phy *s)
{
    uint32_t now = s->bus.clock(s->bus.context);
    if (now < s->last || now - s->start >= s->deadline)
        return 0;
    s->last = now;
    return 1;
}

int ep_step(struct ep_phy *s)
{
    if (!s || !s->initialized)
        return ED_INVALID;
    if (s->phase != EP_RESET && s->phase != EP_RESTART &&
        s->phase != EP_RESTART_WAIT && s->phase != EP_NEGOTIATE)
        return ED_STALE;
    if (!s->remaining || !time_ok(s))
        return failed(s, ED_TIMEOUT);
    s->remaining--;
    int e;
    uint16_t control;
    struct ed_phy result;
    if (s->phase == EP_RESET)
    {
        e = ed_mdio_read(&s->bus, s->hclk, s->address, 0, s->access_polls,
                         s->access_timeout, &control);
        if (e)
            return failed(s, e);
        if (!(control & 0x8000u))
            s->phase = EP_RESTART;
    }
    else if (s->phase == EP_RESTART)
    {
        e = ep_write(&s->bus, s->hclk, s->address, 0, s->access_polls,
                     s->access_timeout, 0x1200u);
        if (e)
            return failed(s, e);
        s->phase = EP_MUTANT == 3 ? EP_NEGOTIATE : EP_RESTART_WAIT;
    }
    else if (s->phase == EP_RESTART_WAIT)
    {
        e = ed_mdio_read(&s->bus, s->hclk, s->address, 0, s->access_polls,
                         s->access_timeout, &control);
        if (e)
            return failed(s, e);
        if (!(control & 0x0200u))
            s->phase = EP_NEGOTIATE;
    }
    else
    {
        e = ed_phy_query(&s->bus, s->hclk, s->address, s->access_polls,
                         s->access_timeout, &result);
        if (e == ED_OK)
        {
            if (!time_ok(s))
                return failed(s, ED_TIMEOUT);
            s->mode = result;
            s->ready = 1;
            s->phase = EP_READY;
            return ED_OK;
        }
        if (e != ED_LINK_DOWN && e != ED_NEGOTIATING)
            return failed(s, e);
    }
    if (!time_ok(s))
        return failed(s, ED_TIMEOUT);
    return ED_BUSY;
}

int ep_poll(struct ep_phy *s)
{
    struct ed_phy result;
    int e;
    if (!s || !s->initialized)
        return ED_INVALID;
    if (s->phase != EP_READY)
        return ED_STALE;
    uint32_t before = s->bus.clock(s->bus.context);
    if (before < s->last)
    {
        e = retire(s);
        return e ? e : ED_TIMEOUT;
    }
    s->last = before;
    e = ed_phy_query(&s->bus, s->hclk, s->address, s->access_polls,
                     s->access_timeout, &result);
    if (e)
    {
        if (EP_MUTANT == 7)
            return e;
        int r = retire(s);
        return r ? r : e;
    }
    uint32_t after = s->bus.clock(s->bus.context);
    if (after < before)
    {
        e = retire(s);
        return e ? e : ED_TIMEOUT;
    }
    s->last = after;
    /* A latch-low event or a different resolved mode invalidates old snapshots,
     * even when the current sample has already recovered to link-up. */
    if ((!result.historical && EP_MUTANT != 6) ||
        result.speed != s->mode.speed || result.full != s->mode.full)
    {
        e = retire(s);
        if (e)
            return e;
    }
    s->mode = result;
    s->ready = 1;
    s->phase = EP_READY;
    return ED_OK;
}

int ep_snapshot(const struct ep_phy *s, struct ep_link *out)
{
    if (!s || !out || !s->initialized)
        return ED_INVALID;
    if (!s->ready || s->phase != EP_READY)
        return ED_STALE;
    struct ep_link result = {s, s->incarnation, s->mode};
    *out = result;
    return ED_OK;
}

int ep_validate(const struct ep_phy *s, const struct ep_link *link)
{
    if (!s || !link || !s->initialized)
        return ED_INVALID;
    return s->ready && s->phase == EP_READY && link->owner == s &&
                   link->incarnation == s->incarnation &&
                   link->mode.speed == s->mode.speed &&
                   link->mode.full == s->mode.full && link->mode.current
               ? ED_OK
               : ED_STALE;
}
