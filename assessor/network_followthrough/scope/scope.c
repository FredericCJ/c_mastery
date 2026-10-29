#include "scope.h"
#ifndef NW_MUTANT
#define NW_MUTANT 0
#endif
int nw_lengths(uint32_t n, struct nw_lengths *out)
{
    if (!out || n > 1460)
    {
        return NW_INVALID;
    }
    uint32_t ip = n + 40, raw = ip + 14, pad = raw < 60 ? 60 - raw : 0;
    *out = (struct nw_lengths){ip,
                               raw,
                               pad,
                               raw + pad,
                               raw + pad + (NW_MUTANT == 1 ? 0 : 4),
                               6 * 2048 + 6 * 16};
    return NW_OK;
}

static int identity(const struct nw_tx *s, struct nw_ticket t)
{
    return t.epoch == s->epoch && (NW_MUTANT == 5 || t.owner == s->owner) &&
           t.generation == s->generation;
}

static int current(const struct nw_tx *s, struct nw_ticket t)
{
    return identity(s, t) && t.serial == s->serial;
}

int nw_tx_init(struct nw_tx *s, uint32_t epoch, uint32_t owner)
{
    if (!s || !epoch || !owner)
    {
        return NW_INVALID;
    }
    *s = (struct nw_tx){.epoch = epoch, .owner = owner};
    return NW_OK;
}

int nw_offer(struct nw_tx *s, const void *data, size_t n)
{
    if (!s || !s->epoch || !s->owner || !data || !n || n > sizeof s->bytes)
    {
        return NW_INVALID;
    }
    if (s->dma || s->prepared || s->acknowledged != s->length)
    {
        return NW_BUSY;
    }
    if (s->generation == UINT32_MAX)
    {
        return NW_RANGE;
    }
    const unsigned char *p = data;
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[i] = p[i];
    }
    ++s->generation;
    s->serial = 0;
    s->length = (uint16_t)n;
    s->sent = s->acknowledged = s->prepared_length = s->flight_end = 0;
    s->prepared = s->flight = s->dma = 0;
    return NW_OK;
}

int nw_prepare(struct nw_tx *s, unsigned mss, unsigned window,
               struct nw_segment *out)
{
    if (!s || !out || !mss || mss > 256 || window > 65535 || !s->epoch)
    {
        return NW_INVALID;
    }
    if (s->prepared || s->flight || s->dma)
    {
        return NW_BUSY;
    }
    if (!window || s->sent == s->length)
    {
        return NW_WAIT;
    }
    if (s->serial == UINT32_MAX)
    {
        return NW_RANGE;
    }
    unsigned n = s->length - s->sent;
    if (n > mss)
    {
        n = mss;
    }
    if (n > window)
    {
        n = window;
    }
    struct nw_segment result = {
        .length = (uint16_t)n,
        .offset = s->sent,
        .ticket = {s->epoch, s->owner, s->generation, s->serial + 1}};
    for (unsigned i = 0; i < n; ++i)
    {
        result.bytes[i] = s->bytes[s->sent + i];
    }
    *out = result;
    ++s->serial;
    s->prepared_length = (uint16_t)n;
    s->prepared = 1;
    return NW_OK;
}

int nw_submit(struct nw_tx *s, struct nw_ticket t, unsigned accepted)
{
    if (!s || accepted > 1)
    {
        return NW_INVALID;
    }
    if (!current(s, t) || !s->prepared)
    {
        return NW_STALE;
    }
    if (accepted || NW_MUTANT == 2)
    {
        s->sent += s->prepared_length;
        s->flight_end = s->sent;
        s->flight = s->dma = 1;
    }
    s->prepared = 0;
    s->prepared_length = 0;
    return NW_OK;
}

int nw_dma_done(struct nw_tx *s, struct nw_ticket t)
{
    if (!s)
    {
        return NW_INVALID;
    }
    if (!current(s, t))
    {
        return NW_STALE;
    }
    if (!s->dma)
    {
        return NW_DUPLICATE;
    }
    s->dma = 0;
    if (NW_MUTANT == 3)
    {
        s->acknowledged = s->sent;
        s->flight = 0;
    }
    return NW_OK;
}

int nw_ack(struct nw_tx *s, struct nw_ticket t, uint32_t n)
{
    if (!s)
    {
        return NW_INVALID;
    }
    if (!identity(s, t))
    {
        return NW_STALE;
    }
    if (n > s->sent)
    {
        return NW_RANGE;
    }
    if (n <= s->acknowledged)
    {
        return NW_DUPLICATE;
    }
    s->acknowledged = (uint16_t)n;
    if (n == s->flight_end)
    {
        s->flight = 0;
    }
    if (NW_MUTANT == 4)
    {
        s->dma = 0;
    }
    return NW_OK;
}

int nw_rx_init(struct nw_rx *s, unsigned capacity)
{
    if (!s || !capacity || capacity > 512)
    {
        return NW_INVALID;
    }
    *s = (struct nw_rx){.capacity = (uint16_t)capacity};
    return NW_OK;
}

int nw_rx_offer(struct nw_rx *s, const void *data, size_t n)
{
    if (!s || (!data && n) || !s->capacity || s->capacity > 512)
    {
        return NW_INVALID;
    }
    if (n > (size_t)(s->capacity - s->count))
    {
        return NW_BUSY;
    }
    const unsigned char *p = data;
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[(s->head + s->count + i) % s->capacity] = p[i];
    }
    if (NW_MUTANT != 6)
    {
        s->count += (uint16_t)n;
    }
    return NW_OK;
}

int nw_rx_peek(const struct nw_rx *s, const unsigned char **out, size_t *length)
{
    if (!s || !out || !length || !s->capacity)
    {
        return NW_INVALID;
    }
    size_t n = s->capacity - s->head;
    if (n > s->count)
    {
        n = s->count;
    }
    *out = s->bytes + s->head;
    *length = n;
    return NW_OK;
}

int nw_rx_consume(struct nw_rx *s, size_t n)
{
    if (!s || !s->capacity)
    {
        return NW_INVALID;
    }
    if (n > s->count)
    {
        return NW_RANGE;
    }
    s->head = (uint16_t)((s->head + n) % s->capacity);
    s->count -= (uint16_t)n;
    return NW_OK;
}

unsigned nw_rx_window(const struct nw_rx *s)
{
    return s && s->capacity ? s->capacity - s->count : 0;
}
