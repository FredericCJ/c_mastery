#include "spi.h"
#ifndef PS_MUTANT
#define PS_MUTANT 0
#endif
static int valid(struct ps_io io)
{
    return io.read && io.write;
}

static int reached(uint32_t now, uint32_t due)
{
    return (uint32_t)(now - due) < UINT32_C(0x80000000);
}

int ps_open(struct ps_spi *s, struct ps_io io, const struct pu_clock *clock,
            unsigned mode, unsigned divider)
{
    struct pu_rates rates;
    if (!s || !valid(io) || !clock || mode > 3 || divider > 7 ||
        pu_rates(clock, &rates) != PU_OK)
        return PS_INVALID;
    if (rates.apb2 > 84000000u || rates.apb2 > (1000000u << (divider + 1u)))
        return PS_INVALID;
    if (s->phase || s->tx_owned || s->rx_owned || s->cs_owned)
        return PS_BUSY;
    if (io.read(io.context, PS_SR) & PS_BSY)
        return PS_BUSY;
    io.write(io.context, PS_CS, 1);
    io.write(io.context, PS_ENABLE, 0);
    io.write(io.context, PS_CONFIG,
             0x304u | (divider << 3) | (PS_MUTANT == 1 ? (mode ^ 1u) : mode));
    io.write(io.context, PS_ENABLE, 1);
    s->ready = 1;
    s->generation = clock->generation;
    s->clock = rates.apb2;
    s->mode = mode;
    s->divider = divider;
    return PS_OK;
}

int ps_begin(struct ps_spi *s, struct ps_io io, uint32_t generation,
             uint32_t token, const uint8_t *tx, uint8_t *rx, unsigned count,
             unsigned dma, uint32_t now, unsigned budget, unsigned setup,
             unsigned hold)
{
    if (!s || !valid(io) || !tx || !rx || !count || count > 16 || dma > 1 ||
        !token || !budget || budget >= UINT32_C(0x80000000) ||
        setup >= budget || hold >= budget)
        return PS_INVALID;
    if (!s->ready || generation != s->generation)
        return PS_STALE;
    if (s->phase || s->tx_owned || s->rx_owned || s->cs_owned)
        return PS_BUSY;
    if (token <= s->last_token)
        return PS_STALE;
    if ((io.read(io.context, PS_SR) &
         (PS_BSY | PS_RXNE | PS_CRCERR | PS_MODF | PS_OVR)) ||
        (dma &&
         (io.read(io.context, PS_TX_EN) || io.read(io.context, PS_RX_EN))))
        return PS_BUSY;
    s->token = s->last_token = token;
    s->last = now;
    s->deadline = now + budget;
    s->due = now + setup;
    s->phase = 1;
    s->dma = dma;
    s->dma_started = 0;
    s->count = count;
    s->sent = s->received = 0;
    s->tx_owned = s->rx_owned = s->cs_owned = 1;
    s->tx = tx;
    s->rx = rx;
    s->hold = hold;
    s->result = PS_BUSY;
    io.write(io.context, PS_CS, 0);
    return PS_OK;
}

int ps_cancel(struct ps_spi *s, struct ps_io io, uint32_t token)
{
    if (!s || !valid(io))
        return PS_INVALID;
    if (!s->phase || s->token != token)
        return PS_STALE;
    if (s->phase == 3)
    {
        if (s->result == PS_BUSY)
            s->result = PS_CANCELLED;
        return PS_BUSY;
    }
    if (s->phase != 4)
    {
        if (s->result == PS_BUSY)
            s->result = PS_CANCELLED;
        if (s->dma && s->dma_started)
        {
            io.write(io.context, PS_DMA_REQUESTS, 0);
            io.write(io.context, PS_TX_EN, 0);
            io.write(io.context, PS_RX_EN, 0);
        }
        else
        {
            s->tx_owned = s->rx_owned = 0;
        }
        s->phase = 4;
    }
    if (PS_MUTANT == 4)
        s->tx_owned = s->rx_owned = 0;
    return PS_BUSY;
}

static int finish(struct ps_spi *s, struct ps_io io)
{
    io.write(io.context, PS_CS, 1);
    s->cs_owned = 0;
    s->phase = 0;
    return s->result == PS_BUSY ? (s->result = PS_OK) : s->result;
}

int ps_abort_reset(struct ps_spi *s, struct ps_io io, uint32_t token,
                   uint32_t now)
{
    uint32_t sr;
    if (!s || !valid(io))
        return PS_INVALID;
    if (!s->phase || s->token != token)
        return PS_STALE;
    if ((uint32_t)(now - s->last) >= UINT32_C(0x80000000))
        return PS_INVALID;
    s->last = now;
    if (s->phase != 4)
        return PS_BUSY;
    if (s->dma && s->dma_started &&
        (io.read(io.context, PS_TX_EN) || io.read(io.context, PS_RX_EN)))
        return PS_BUSY;
    s->tx_owned = s->rx_owned = 0;
    if (s->dma && s->dma_started)
    {
        unsigned tx = io.read(io.context, PS_TX_COUNT),
                 rx = io.read(io.context, PS_RX_COUNT);
        if (tx <= s->count)
            s->sent = s->count - tx;
        else
            s->result = PS_IO;
        if (rx <= s->count)
            s->received = s->count - rx;
        else
            s->result = PS_IO;
    }
    io.write(io.context, PS_RESET, 1);
    s->ready = 0;
    sr = io.read(io.context, PS_SR);
    if (!(sr & PS_TXE) || (sr & PS_BSY))
        return PS_BUSY;
    s->due = s->last + s->hold;
    s->phase = 3;
    return PS_BUSY;
}

int ps_step(struct ps_spi *s, struct ps_io io, uint32_t token, uint32_t now)
{
    uint32_t sr;
    if (!s || !valid(io))
        return PS_INVALID;
    if (!s->phase || s->token != token)
        return PS_STALE;
    if ((uint32_t)(now - s->last) >= UINT32_C(0x80000000))
        return PS_INVALID;
    s->last = now;
    sr = io.read(io.context, PS_SR);
    if ((sr & (PS_CRCERR | PS_MODF | PS_OVR)) && s->phase != 4)
    {
        s->result = PS_IO;
        ps_cancel(s, io, token);
    }
    if (s->phase == 1 && reached(now, s->due))
    {
        if (s->dma)
        {
            io.write(io.context, PS_DMA_CLEAR, 1);
            io.write(io.context, PS_TX_ADDR, (uintptr_t)s->tx);
            io.write(io.context, PS_RX_ADDR, (uintptr_t)s->rx);
            io.write(io.context, PS_TX_COUNT, s->count);
            io.write(io.context, PS_RX_COUNT, s->count);
            io.write(io.context, PS_RX_START, 1);
            io.write(io.context, PS_DMA_REQUESTS, 3);
            io.write(io.context, PS_TX_START, 1);
            s->dma_started = 1;
        }
        s->phase = 2;
    }
    if (s->phase == 2)
    {
        if (s->dma)
        {
            unsigned te = io.read(io.context, PS_TX_EN),
                     re = io.read(io.context, PS_RX_EN),
                     tf = io.read(io.context, PS_TX_FLAGS),
                     rf = io.read(io.context, PS_RX_FLAGS);
            if ((tf | rf) & 2u)
            {
                s->result = PS_IO;
                ps_cancel(s, io, token);
            }
            else
            {
                if (!te)
                {
                    s->tx_owned = 0;
                    if ((tf & 1u) && !io.read(io.context, PS_TX_COUNT))
                        s->sent = s->count;
                    else
                    {
                        s->result = PS_IO;
                        ps_cancel(s, io, token);
                    }
                }
                if (!re)
                {
                    s->rx_owned = 0;
                    if ((rf & 1u) && !io.read(io.context, PS_RX_COUNT))
                        s->received = s->count;
                    else
                    {
                        s->result = PS_IO;
                        ps_cancel(s, io, token);
                    }
                }
            }
            /* DMA can finish its memory accesses after the initial SR snapshot.
               Observe wire completion after these acknowledgements. */
            if (PS_MUTANT != 7)
                sr = io.read(io.context, PS_SR);
        }
        else
        {
            if (s->sent > s->received && (sr & PS_RXNE))
            {
                s->rx[s->received++] =
                    PS_MUTANT == 2 ? 0 : (uint8_t)io.read(io.context, PS_DR);
                if (s->received == s->count)
                    s->rx_owned = 0;
            }
            if (s->sent == s->received && s->sent < s->count && (sr & PS_TXE))
            {
                io.write(io.context, PS_DR, s->tx[s->sent++]);
                if (s->sent == s->count)
                    s->tx_owned = 0;
                /* No pre-write BSY/TXE snapshot may acknowledge this write. */
                sr = PS_BSY;
            }
        }
        if (s->phase == 2 && s->sent == s->count && s->received == s->count &&
            !s->tx_owned && !s->rx_owned && (sr & PS_TXE) &&
            (!(sr & PS_BSY) || PS_MUTANT == 3))
        {
            if (s->dma)
                io.write(io.context, PS_DMA_REQUESTS, 0);
            s->due = now + s->hold;
            s->phase = 3;
        }
    }
    if (s->phase == 4)
    {
        if (s->dma && s->dma_started)
        {
            if (!io.read(io.context, PS_TX_EN))
            {
                unsigned left = io.read(io.context, PS_TX_COUNT);
                s->tx_owned = 0;
                if (left <= s->count)
                    s->sent = s->count - left;
                else
                    s->result = PS_IO;
            }
            if (!io.read(io.context, PS_RX_EN))
            {
                unsigned left = io.read(io.context, PS_RX_COUNT);
                s->rx_owned = 0;
                if (left <= s->count)
                    s->received = s->count - left;
                else
                    s->result = PS_IO;
            }
        }
        /* Draining is not a promise that a slave accepted any command. */
        if (!s->rx_owned && (sr & PS_RXNE))
            (void)io.read(io.context, PS_DR);
        if (!s->tx_owned && !s->rx_owned && (sr & PS_TXE) && !(sr & PS_BSY))
        {
            s->due = now + s->hold;
            s->phase = 3;
        }
    }
    if (s->phase == 3 && reached(now, s->due))
        return finish(s, io);
    if (s->result == PS_BUSY && reached(now, s->deadline))
    {
        s->result = PS_TIMEOUT;
        ps_cancel(s, io, token);
    }
    return PS_BUSY;
}

int ps_command(unsigned version, unsigned reg, uint8_t out[3])
{
    if (!out || version > 1 || reg > 31)
        return PS_INVALID;
    out[0] = (uint8_t)((version ? 0xc0u : 0x80u) | reg);
    out[1] = 0;
    out[2] = 0;
    return PS_OK;
}

int ps_decode(unsigned version, const uint8_t in[3], uint16_t *out)
{
    uint16_t value;
    if (!in || !out || version > 1)
        return PS_INVALID;
    if ((in[0] != (version ? 0xa6u : 0xa5u)) && PS_MUTANT != 5)
        return PS_PROTOCOL;
    value = version ? (uint16_t)((uint16_t)in[2] << 8 | in[1])
                    : (uint16_t)((uint16_t)in[1] << 8 | in[2]);
    if (PS_MUTANT == 6)
        value = (uint16_t)((uint16_t)in[1] << 8 | in[2]);
    *out = value;
    return PS_OK;
}

int ps_pack(const uint8_t *src, uint16_t *dst, unsigned count)
{
    if (!src || !dst || !count || count > 16)
        return PS_INVALID;
    for (unsigned i = 0; i < count; i++)
    {
        dst[i] = src[i];
    }
    return PS_OK;
}

int ps_unpack(const uint16_t *src, uint8_t *dst, unsigned count)
{
    if (!src || !dst || !count || count > 16)
        return PS_INVALID;
    for (unsigned i = 0; i < count; i++)
    {
        dst[i] = (uint8_t)src[i];
    }
    return PS_OK;
}
