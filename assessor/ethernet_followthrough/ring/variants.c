#include "variants.h"
#ifndef EV_MUTANT
#define EV_MUTANT 0
#endif
int ev_order(const unsigned char events[8], uint32_t *out)
{
    if (!events || !out)
        return ED_INVALID;
    unsigned seen = 0, pos[8];
    for (unsigned i = 0; i < 8; i++)
    {
        unsigned e = events[i];
        if (e >= 8 || (seen & (1u << e)))
            return ED_INVALID;
        seen |= 1u << e;
        pos[e] = i;
    }
    const unsigned from[] = {0, 1, 2, 1, 4, 5, 6}, to[] = {1, 2, 3, 7, 5, 7, 7};
    uint32_t bad = 0;
    for (unsigned i = 0; i < 7; i++)
        if (pos[from[i]] >= pos[to[i]])
            bad |= 1u << i;
    if (EV_MUTANT == 5)
        bad = 0;
    *out = bad;
    return bad ? ED_INVALID : ED_OK;
}

int ev_prepare(struct ev_chain *s, const unsigned char *p, size_t n,
               unsigned count, uint32_t token)
{
    if (!s || !p || n < 14 || n > 1514 || count < 2 || count > 3 || !token)
        return ED_INVALID;
    if (s->borrow)
        return ED_BUSY;
    if (token <= s->token)
        return ED_STALE;
    struct ev_chain next = {p, n, token, count, (1u << count) - 1u, 0, 0};
    *s = next;
    return ED_OK;
}

int ev_own(struct ev_chain *s, unsigned i)
{
    if (!s || !s->borrow || i >= s->count)
        return ED_INVALID;
    if (s->aborting || (s->owned & (1u << i)))
        return ED_BUSY;
    unsigned all = (1u << s->count) - 1u;
    if (s->prepared != all ||
        (i == 0 && s->owned != (all & ~1u) && EV_MUTANT != 1))
        return ED_BUSY;
    s->owned |= 1u << i;
    return ED_OK;
}

static void end(struct ev_chain *s)
{
    s->borrow = 0;
    s->length = 0;
    s->prepared = s->owned = s->aborting = 0;
}

int ev_complete(struct ev_chain *s, unsigned returned, unsigned error)
{
    if (!s || returned > 1 || error > 1)
        return ED_INVALID;
    if (!s->borrow || s->aborting || s->owned != (1u << s->count) - 1u)
        return ED_STALE;
    if (!returned && EV_MUTANT != 2)
        return ED_BUSY;
    end(s);
    return error ? ED_IO : ED_OK;
}

int ev_abort(struct ev_chain *s, unsigned stopped)
{
    if (!s || stopped > 1)
        return ED_INVALID;
    if (!s->borrow)
        return ED_STALE;
    s->aborting = 1;
    if (!stopped)
        return ED_BUSY;
    end(s);
    return ED_OK;
}

int ev_span(const unsigned char *p, size_t n, unsigned cstf, size_t *out)
{
    if (!p || !out || n < 14 || n > 1518 || cstf > 1)
        return ED_INVALID;
    unsigned field = (unsigned)p[12] * 256u + p[13];
    unsigned stripped = cstf && field > 0x600u;
    if (n < (stripped ? 60u : 64u) || n > (stripped ? 1514u : 1518u))
        return ED_INVALID;
    *out = n - ((stripped && EV_MUTANT != 3) ? 0u : 4u);
    return ED_OK;
}

int ev_store(struct ev_visibility *s, unsigned i, unsigned char v)
{
    if (!s || i >= 64)
        return ED_INVALID;
    unsigned bit = 1u << (i / 32);
    if (s->device_owned & bit)
        return ED_BUSY;
    s->cpu[i] = v;
    s->dirty |= bit;
    return ED_OK;
}

int ev_clean(struct ev_visibility *s, unsigned line)
{
    if (!s || line > 1)
        return ED_INVALID;
    unsigned bit = 1u << line;
    if (s->device_owned & bit)
        return ED_BUSY;
    if (EV_MUTANT != 4)
        for (unsigned i = line * 32; i < (line + 1) * 32; i++)
            s->device[i] = s->cpu[i];
    s->dirty &= ~bit;
    return ED_OK;
}

int ev_publish(struct ev_visibility *s, unsigned line)
{
    if (!s || line > 1)
        return ED_INVALID;
    unsigned bit = 1u << line;
    if ((s->device_owned | s->dirty) & bit)
        return ED_BUSY;
    s->device_owned |= bit;
    return ED_OK;
}

int ev_device_store(struct ev_visibility *s, unsigned i, unsigned char v)
{
    if (!s || i >= 64)
        return ED_INVALID;
    if (!(s->device_owned & (1u << (i / 32))))
        return ED_BUSY;
    s->device[i] = v;
    return ED_OK;
}

int ev_return(struct ev_visibility *s, unsigned line, unsigned returned)
{
    if (!s || line > 1 || returned > 1)
        return ED_INVALID;
    unsigned bit = 1u << line;
    if (!(s->device_owned & bit))
        return ED_STALE;
    if (!returned)
        return ED_BUSY;
    s->device_owned &= ~bit;
    return ED_OK;
}

int ev_invalidate(struct ev_visibility *s, unsigned line)
{
    if (!s || line > 1)
        return ED_INVALID;
    unsigned bit = 1u << line;
    if ((s->device_owned | s->dirty) & bit)
        return ED_BUSY;
    for (unsigned i = line * 32; i < (line + 1) * 32; i++)
        s->cpu[i] = s->device[i];
    return ED_OK;
}
