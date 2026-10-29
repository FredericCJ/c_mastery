#include "sync.h"
#include <limits.h>
#ifndef SU_MUTANT
#define SU_MUTANT 0
#endif
static int io_valid(struct pu_io io)
{
    return io.read && io.write;
}

int su_configure(struct mc_su *s, struct pu_io io, unsigned p, unsigned h,
                 unsigned l)
{
    if (!s || !io_valid(io) || p > 1 || h > 1 || l > 1 || !s->byte.ready)
        return PU_INVALID;
    if (s->active || s->byte.active || s->byte.memory_owned ||
        s->byte.line_owned)
        return PU_BUSY;
    if (SU_MUTANT != 1 && (io.read(io.context, PU_CR1) & 12u))
        return PU_BUSY;
    io.write(io.context, PU_CR3, 0);
    io.write(io.context, PU_CR2, 0x800u | p << 10 | h << 9 | l << 8);
    s->cpol = p;
    s->cpha = h;
    s->lbcl = l;
    s->configured = 1;
    return PU_OK;
}

int su_enable(struct mc_su *s, struct pu_io io, unsigned tx, unsigned rx)
{
    if (!s || !io_valid(io) || !s->configured || tx > 1 || rx > 1)
        return PU_INVALID;
    if (s->active || s->byte.active || s->byte.memory_owned ||
        s->byte.line_owned)
        return PU_BUSY;
    uint32_t cr1 = io.read(io.context, PU_CR1);
    io.write(io.context, PU_CR1, (cr1 & ~12u) | tx << 3 | rx << 2);
    return PU_OK;
}

static int next_byte(struct mc_su *s, struct pu_io io, uint32_t now)
{
    if (s->byte.token == UINT32_MAX)
        return PU_STALE;
    return pu_submit(&s->byte, io, s->byte.configured_generation,
                     s->byte.token + 1, PU_POLL, s->tx + s->position, 1, now,
                     s->deadline - now);
}

int su_begin(struct mc_su *s, struct pu_io io, uint32_t gen, uint32_t token,
             const uint8_t *tx, unsigned n, struct pu_byte *rx, unsigned cap,
             uint32_t now, uint32_t budget)
{
    if (!s || !io_valid(io) || !tx || !rx || !token || !n || n > 8 || cap < n ||
        cap > 8 || !budget || budget >= 0x80000000u || !s->configured)
        return PU_INVALID;
    if (s->active || s->byte.active || s->byte.memory_owned ||
        s->byte.line_owned)
        return PU_BUSY;
    if (!s->byte.ready || gen != s->byte.configured_generation ||
        token <= s->token || n > UINT32_MAX - s->byte.token)
        return PU_STALE;
    if (!s->lbcl)
        return SU_UNSUPPORTED;
    if (SU_MUTANT != 2 && (io.read(io.context, PU_CR1) & 0x200cu) != 0x200cu)
        return SU_UNSUPPORTED;
    /* Idle, known peer: no pending received byte may be mistaken for this request. */
    if (io.read(io.context, PU_SR) & PU_RXNE)
        return PU_BUSY;
    int r = pu_submit(&s->byte, io, gen, s->byte.token + 1, PU_POLL, tx, 1, now,
                      budget);
    if (r)
        return r;
    s->token = token;
    s->deadline = now + budget;
    s->last = now;
    s->active = 1;
    s->count = n;
    s->position = 0;
    s->received = 0;
    s->byte_received = 0;
    s->stopping = 0;
    s->result = PU_BUSY;
    s->tx = tx;
    s->rx = rx;
    return PU_OK;
}

int su_cancel(struct mc_su *s, struct pu_io io, uint32_t token)
{
    if (!s || !io_valid(io))
        return PU_INVALID;
    if (!s->active || s->token != token)
        return PU_STALE;
    s->stopping = 1;
    if (s->result == PU_BUSY)
        s->result = PU_CANCELLED;
    if (s->byte.active)
        return pu_cancel(&s->byte, io, s->byte.token);
    return PU_OK;
}

int su_step(struct mc_su *s, struct pu_io io, uint32_t token, uint32_t now)
{
    if (!s || !io_valid(io))
        return PU_INVALID;
    if (!s->active || token != s->token)
        return PU_STALE;
    if (now - s->last >= 0x80000000u)
        return PU_INVALID;
    s->last = now;
    if (!s->stopping && !s->byte_received)
    {
        int r = pu_rx_poll(&s->receive, io, 1);
        if (r == PU_OK)
        {
            struct pu_byte b;
            if (pu_take(&s->receive, &b))
                return PU_IO;
            s->rx[s->received++] = b;
            s->byte_received = 1;
            if (b.error && SU_MUTANT != 5)
            {
                s->result = PU_IO;
                (void)su_cancel(s, io, token);
            }
        }
        else if (r != PU_TIMEOUT)
        {
            s->result = PU_IO;
            (void)su_cancel(s, io, token);
        }
    }
    if (s->byte.active)
    {
        int r = pu_step(&s->byte, io, s->byte.token, now);
        if (r != PU_OK && r != PU_BUSY)
        {
            if (s->result == PU_BUSY)
                s->result = (unsigned)r;
            s->stopping = 1;
        }
    }
    if (SU_MUTANT == 3)
        s->byte.line_owned = 0;
    if (SU_MUTANT == 3)
        s->byte.active = 0;
    if (!s->stopping && !s->byte.active && s->byte_received)
    {
        s->position++;
        if (s->position == s->count)
        {
            s->active = 0;
            s->result = PU_OK;
            return PU_OK;
        }
        if (now - s->deadline >= 0x80000000u)
        {
            s->byte_received = 0;
            int r = next_byte(s, io, now);
            if (r)
            {
                s->result = (unsigned)r;
                s->stopping = 1;
            }
        }
        else
        {
            s->result = PU_TIMEOUT;
            (void)su_cancel(s, io, token);
        }
    }
    if (!s->stopping && now - s->deadline < 0x80000000u)
    {
        s->result = PU_TIMEOUT;
        (void)su_cancel(s, io, token);
    }
    if (s->stopping && !s->byte.active && !s->byte.memory_owned &&
        !s->byte.line_owned)
    {
        /* TC established quiescent transmit; stop receive before releasing its span. */
        io.write(io.context, PU_CR1, io.read(io.context, PU_CR1) & ~12u);
        unsigned sr = io.read(io.context, PU_SR);
        if (sr & PU_RXNE)
            (void)io.read(io.context, PU_DR);
        s->active = 0;
        return (int)s->result;
    }
    return PU_BUSY;
}

int su_trace(unsigned p, unsigned h, unsigned l, const struct sd_sync *t,
             unsigned *bad)
{
    if (!t || !bad || p > 1 || h > 1 || l > 1)
        return PU_INVALID;
    unsigned n = l ? 16 : 14;
    for (unsigned i = 0; i < n && i < t->edges; i++)
    {
        if (t->level[i] != (p ^ ((i + 1u) & 1u)) ||
            t->sample[i] != ((i & 1u) == h))
        {
            *bad = i + 1;
            return PU_IO;
        }
    }
    if (t->edges != n)
    {
        *bad = (t->edges < n ? t->edges : n) + 1u;
        return PU_IO;
    }
    if (SU_MUTANT == 4)
    {
        *bad = 1;
        return PU_IO;
    }
    *bad = 0;
    return PU_OK;
}
