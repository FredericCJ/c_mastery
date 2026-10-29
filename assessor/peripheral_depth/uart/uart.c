#include "uart.h"
#include "serial_depth.h"
#include <limits.h>
#ifndef PU_MUTANT
#define PU_MUTANT 0
#endif
static int apb(unsigned n)
{
    return n == 1 || n == 2 || n == 4 || n == 8 || n == 16;
}

static int valid(struct pu_io io)
{
    return io.read && io.write;
}

int pu_rates(const struct pu_clock *c, struct pu_rates *r)
{
    if (!c || !r || !c->generation || !c->sys_hz || c->sys_hz > 180000000u ||
        !(apb(c->ahb) || c->ahb == 64 || c->ahb == 128 || c->ahb == 256 ||
          c->ahb == 512) ||
        !apb(c->apb1) || !apb(c->apb2) || c->timpre > 1 || c->sys_hz % c->ahb)
        return PU_INVALID;
    uint32_t h = c->sys_hz / c->ahb;
    if (h % c->apb1 || h % c->apb2 || h / c->apb1 > 45000000u ||
        h / c->apb2 > 90000000u)
        return PU_INVALID;
    r->core = h;
    r->apb1 = h / c->apb1;
    r->apb2 = h / c->apb2;
    r->timer1 = c->timpre ? (c->apb1 <= 4 ? h : 4 * r->apb1)
                          : (c->apb1 == 1 ? r->apb1 : 2 * r->apb1);
    r->timer2 = c->timpre ? (c->apb2 <= 4 ? h : 4 * r->apb2)
                          : (c->apb2 == 1 ? r->apb2 : 2 * r->apb2);
    if (PU_MUTANT == 1)
        r->timer1 = 2 * r->apb1;
    return PU_OK;
}

int pu_peer_ready(const struct pu_rx *q, unsigned *ready)
{
    if (!q || !ready || q->count > 2 || q->head > 1 || q->backpressure > 1 ||
        q->reserve > 2)
        return PU_INVALID;
    *ready = !q->backpressure || 2 - q->count > q->reserve;
    return PU_OK;
}

int pu_receive(struct pu_rx *q, unsigned status, uint8_t value)
{
    unsigned ready;
    if (!q || status > 255 || pu_peer_ready(q, &ready))
        return PU_INVALID;
    unsigned error = status & 15u;
    if (!(status & PU_RXNE))
        return PU_BUSY;
    if (error)
    {
        if (q->errors == UINT32_MAX)
            return PU_FULL;
        q->errors++;
    }
    if (q->count == 2)
    {
        if (q->dropped != UINT32_MAX)
            q->dropped++;
        return PU_FULL;
    }
    unsigned at = (q->head + q->count) % 2;
    q->items[at].value = value;
    q->items[at].error = (uint8_t)(PU_MUTANT == 4 ? 0 : error);
    q->count++;
    return PU_OK;
}

int pu_take(struct pu_rx *q, struct pu_byte *v)
{
    unsigned ready;
    if (!v || pu_peer_ready(q, &ready))
        return PU_INVALID;
    if (!q->count)
        return PU_BUSY;
    *v = q->items[q->head];
    q->head = (q->head + 1) % 2;
    q->count--;
    return PU_OK;
}

int pu_open(struct pu_driver *d, struct pu_io io, const struct pu_clock *c,
            uint32_t baud, unsigned over8, unsigned polls)
{
    struct pu_rates rates;
    struct sd_uart frame;
    if (!d || !valid(io) || !polls || pu_rates(c, &rates) ||
        sd_uart(rates.apb1, baud, over8, 8, 0, 1, 0, &frame))
        return PU_INVALID;
    if (d->active || d->memory_owned || d->line_owned)
        return PU_BUSY;
    d->ready = 0;
    while (polls--)
    {
        if (io.read(io.context, PU_SOURCE))
            goto source_ready;
    }
    return PU_TIMEOUT;
source_ready:
    io.write(io.context, PU_GATE, 1);
    if (PU_MUTANT != 2)
        (void)io.read(io.context, PU_GATE);
    io.write(io.context, PU_RESET, 1);
    io.write(io.context, PU_RESET, 0);
    io.write(io.context, PU_CR1, 1u << 13);
    io.write(io.context, PU_CR2, 0);
    io.write(io.context, PU_CR3, 0);
    io.write(io.context, PU_BRR, PU_MUTANT == 3 ? frame.divisor : frame.brr);
    io.write(io.context, PU_CR1, (1u << 13) | 12u | (over8 << 15));
    d->configured_generation = c->generation;
    d->ready = 1;
    return PU_OK;
}

int pu_submit(struct pu_driver *d, struct pu_io io, uint32_t gen,
              uint32_t token, unsigned backend, const uint8_t *buf, unsigned n,
              uint32_t now, uint32_t budget)
{
    if (!d || !valid(io) || !buf || !n || n > 8 || backend > PU_DMA || !token ||
        !budget || budget >= 0x80000000u)
        return PU_INVALID;
    if (!d->ready || d->active || d->memory_owned || d->line_owned)
        return PU_BUSY;
    if ((PU_MUTANT != 5 && gen != d->configured_generation) ||
        token <= d->token)
        return PU_STALE;
    if (backend == PU_DMA)
    {
        io.write(io.context, PU_DMA_GATE, 1);
        (void)io.read(io.context, PU_DMA_GATE);
        if (io.read(io.context, PU_DMA_BUSY))
            return PU_BUSY;
    }
    d->token = token;
    d->backend = backend;
    d->count = n;
    d->position = 0;
    d->buffer = buf;
    d->active = 1;
    d->memory_owned = 1;
    d->line_owned = 1;
    d->cancel = 0;
    d->result = PU_BUSY;
    d->last = now;
    d->deadline = now + budget;
    if (backend == PU_DMA)
    {
        io.write(io.context, PU_DMA_CLEAR, 1);
        io.write(io.context, PU_DMA_ADDRESS, (uintptr_t)buf);
        io.write(io.context, PU_DMA_COUNT, n);
        /* Only writable flags receive ones; clear TC without clearing RXNE/CTS/LBD.
         */
        io.write(io.context, PU_SR, 0x320u);
        io.write(io.context, PU_DMA_CONTROL, 1);
        io.write(io.context, PU_CR3, 1u << 7);
    }
    else if (backend == PU_IRQ)
        io.write(io.context, PU_CR1,
                 (io.read(io.context, PU_CR1) | 0x80u) & ~0x40u);
    return PU_OK;
}

int pu_cancel(struct pu_driver *d, struct pu_io io, uint32_t token)
{
    if (!d || !valid(io))
        return PU_INVALID;
    if (!d->active || token != d->token)
        return PU_STALE;
    d->cancel = 1;
    if (d->result == PU_BUSY)
        d->result = PU_CANCELLED;
    if (d->backend == PU_DMA)
        io.write(io.context, PU_DMA_CONTROL, 0);
    else
    {
        d->memory_owned = 0;
        io.write(io.context, PU_CR1, io.read(io.context, PU_CR1) & ~0x80u);
    }
    if (PU_MUTANT == 6)
        d->memory_owned = 0;
    return PU_OK;
}

int pu_reset_notice(struct pu_driver *d, struct pu_io io)
{
    if (!d || !valid(io))
        return PU_INVALID;
    d->ready = 0;
    if (d->active)
    {
        d->result = PU_IO;
        return pu_cancel(d, io, d->token);
    }
    return PU_OK;
}

int pu_step(struct pu_driver *d, struct pu_io io, uint32_t token, uint32_t now)
{
    if (!d || !valid(io))
        return PU_INVALID;
    if (!d->active || token != d->token)
        return PU_STALE;
    if (now - d->last >= 0x80000000u)
        return PU_INVALID;
    d->last = now;
    if (d->backend == PU_DMA)
    {
        unsigned flags = io.read(io.context, PU_DMA_FLAGS);
        if (flags & 2u)
        {
            d->result = PU_IO;
            (void)pu_cancel(d, io, token);
        }
        if (!io.read(io.context, PU_DMA_BUSY))
        {
            if (!d->cancel &&
                (!(flags & 1u) || io.read(io.context, PU_DMA_COUNT)))
            {
                d->result = PU_IO;
                d->cancel = 1;
            }
            d->memory_owned = 0;
            d->position = d->cancel ? d->position : d->count;
            io.write(io.context, PU_CR3, 0);
        }
    }
    unsigned sr = io.read(io.context, PU_SR);
    if (d->backend != PU_DMA && !d->cancel && d->position < d->count &&
        (sr & PU_TXE))
    {
        io.write(io.context, PU_DR, d->buffer[d->position++]);
        if (d->position == d->count)
        {
            d->memory_owned = 0;
            if (d->backend == PU_IRQ)
                io.write(io.context, PU_CR1,
                         (io.read(io.context, PU_CR1) & ~0x80u) | 0x40u);
        }
        /* This snapshot precedes the DR write; its TC cannot acknowledge that
         * write. */
        sr &= ~PU_TC;
    }
    if (PU_MUTANT == 7 && sr & PU_TXE)
        sr |= PU_TC;
    if (!d->memory_owned && (sr & PU_TC))
    {
        d->line_owned = 0;
        d->active = 0;
        io.write(io.context, PU_CR1, io.read(io.context, PU_CR1) & ~0xc0u);
        if (d->result == PU_BUSY)
            d->result = PU_OK;
        return (int)d->result;
    }
    if (!d->cancel && now - d->deadline < 0x80000000u)
    {
        d->result = PU_TIMEOUT;
        (void)pu_cancel(d, io, token);
    }
    return PU_BUSY;
}

int pu_poll(struct pu_driver *d, struct pu_io io, uint32_t token, uint32_t now,
            unsigned polls)
{
    if (!polls)
        return PU_INVALID;
    while (polls--)
    {
        int result = pu_step(d, io, token, now);
        if (result != PU_BUSY)
            return result;
    }
    /* Poll exhaustion is a separate cancellation trigger, not an elapsed-time
     * claim. */
    if (d && d->active)
    {
        d->result = PU_TIMEOUT;
        (void)pu_cancel(d, io, token);
    }
    return PU_TIMEOUT;
}

int pu_rx_poll(struct pu_rx *q, struct pu_io io, unsigned polls)
{
    unsigned ready;
    if (!valid(io) || !polls || pu_peer_ready(q, &ready))
        return PU_INVALID;
    while (polls--)
    {
        unsigned sr = io.read(io.context, PU_SR);
        if (sr & PU_RXNE)
        {
            uint8_t b = (uint8_t)io.read(io.context, PU_DR);
            return pu_receive(q, sr & 255u, b);
        }
    }
    return PU_TIMEOUT;
}

int pu_deadline_fit(uint32_t clock, uint32_t divisor, unsigned n, uint32_t us,
                    unsigned *fits)
{
    if (!fits || !clock || clock > 45000000u || !divisor || divisor > 65535 ||
        !n || n > 8 || us > 1000000u)
        return PU_INVALID;
    /* 8N1: bytes*10 bits. 64-bit products fit under these explicit bounds. */
    *fits = (uint64_t)n * 10 * divisor * 1000000u <= (uint64_t)clock * us;
    return PU_OK;
}
