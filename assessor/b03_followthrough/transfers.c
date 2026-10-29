#include "follow.h"
#include <limits.h>
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_turn(struct bf_round *r, b3_write_fn f[2], void *c[2], unsigned quota,
            unsigned *chosen)
{
    unsigned n;
    if (!r || !f || !c || !chosen || r->next > 1 || !f[0] || !f[1] ||
        quota > 64)
        return B3_INVALID;
    n = r->next;
    *chosen = n;
#if BF_MUTANT != 1
    r->next = 1 - n;
#endif
    return b3_pump(&r->writer[n], f[n], c[n], quota);
}

int bf_message(struct bf_message *m, unsigned id, const unsigned char *p,
               unsigned n, unsigned quota, int lose, unsigned *out)
{
    unsigned i;
    if (!m || !out || (!p && n) || n > 6 || quota > 6 ||
        (lose != 0 && lose != 1) || !id || id > 3)
        return B3_INVALID;
    if (id == m->id)
    {
        if (n != m->length)
            return B3_FAILED;
        for (i = 0; i < n; i++)
            if (p[i] != m->request[i])
                return B3_FAILED;
#if BF_MUTANT == 2
        m->done = 0;
#endif
    }
    else
    {
        if (id != m->id + 1 || (!m->complete && m->id))
            return B3_STALE;
        m->id = id;
        m->length = n;
        m->done = 0;
        m->complete = 0;
        for (i = 0; i < n; i++)
            m->request[i] = p[i];
    }
    for (i = 0; i < quota && m->done < n; i++)
    {
        /* One atomic model transition: application AND retained progress. */
        m->total += m->request[m->done];
        m->done++;
        m->effects++;
    }
    if (m->done == n)
        m->complete = 1;
    if (!m->complete || lose)
        return B3_WAIT;
    *out = m->total;
    return B3_OK;
}

int bf_start(struct bf_operation *o)
{
    if (!o)
        return B3_INVALID;
    if (o->owned || o->generation == 3)
        return B3_FAILED;
    o->generation++;
    o->phase = BF_ACTIVE;
    o->owned = 1;
    return B3_OK;
}

int bf_event(struct bf_operation *o, unsigned generation, enum bf_event e)
{
    if (!o || e < BF_TIMEOUT || e > BF_QUIESCENT)
        return B3_INVALID;
    if (generation != o->generation || !o->owned)
        return B3_STALE;
    if (e == BF_TIMEOUT)
    {
        if (o->phase == BF_ACTIVE)
            o->phase = BF_REQUESTED;
        return B3_WAIT;
    }
    if (e == BF_REJECT)
    {
        if (o->phase != BF_REQUESTED)
            return B3_STALE;
        o->phase = BF_ACTIVE;
        return B3_WAIT;
    }
    if (e == BF_QUEUED_ACK)
    {
        if (o->phase != BF_REQUESTED)
            return B3_STALE;
        o->phase = BF_QUEUED;
#if BF_MUTANT == 3
        o->owned = 0;
        o->reports++;
#endif
        return B3_WAIT;
    }
    if (e == BF_QUIESCENT && o->phase != BF_QUEUED)
        return B3_STALE;
    o->phase = e == BF_COMPLETE ? BF_DONE : BF_CANCELLED;
    o->owned = 0;
    o->reports++;
    return B3_OK;
}

int bf_clear_read(struct bf_register *r, unsigned *out)
{
    if (!r || !out)
        return B3_INVALID;
    *out = r->status;
    r->status = 0;
    r->reads++;
    return B3_OK;
}

int bf_w0c(struct bf_register *r, unsigned mask)
{
    unsigned wire;
    if (!r || mask > 15)
        return B3_INVALID;
    wire = 255u ^ mask;
#if BF_MUTANT == 4
    wire = mask;
#endif
    r->last_write = wire;
    r->writes++;
    r->status &= wire;
    return B3_OK;
}

int bf_coherent(bf_read_half_fn read, void *ctx, unsigned budget, uint16_t *out)
{
    unsigned a, b, lo, i;
    if (!read || !out || budget > 8)
        return B3_INVALID;
    for (i = 0; i < budget; i++)
    {
        a = read(ctx, 1);
        lo = read(ctx, 0);
        b = read(ctx, 1);
        if (a > 255 || b > 255 || lo > 255)
            return B3_FAILED;
#if BF_MUTANT == 5
        b = a;
#endif
        if (a == b)
        {
            *out = (uint16_t)(a * 256 + lo);
            return B3_OK;
        }
    }
    return B3_WAIT;
}

int bf_due16(uint16_t now, uint16_t start, uint16_t delay, int *out)
{
    unsigned e = (uint16_t)(now - start);
    if (!out || delay >= 32768)
        return B3_INVALID;
    if (e >= 32768)
        return B3_AMBIGUOUS;
    *out = e >= delay;
    return B3_OK;
}

int bf_ticks(unsigned ms, unsigned num, unsigned den, unsigned *out)
{
    unsigned product, q;
    if (!out || !num || !den || ms > UINT_MAX / den)
        return B3_INVALID;
    product = ms * den;
    q = product / num;
    if (product % num)
    {
        if (q == UINT_MAX)
            return B3_INVALID;
        q++;
    }
    *out = q;
    return B3_OK;
}
