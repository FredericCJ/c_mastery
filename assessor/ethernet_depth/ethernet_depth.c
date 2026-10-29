#include "ethernet_depth.h"
#include <limits.h>

int ed_wrap(uint32_t app, const uint32_t *h, size_t n, uint32_t *out)
{
    uint32_t t[4];
    size_t i;
    if (n > 4 || (!h && n) || (!out && n))
        return ED_INVALID;
    for (i = 0; i < n; i++)
    {
        if (h[i] > UINT32_MAX - app)
            return ED_INVALID;
        app += h[i];
        t[i] = app;
    }
#ifdef ED_MUTATE_LAYERS
    if (n)
        t[n - 1] -= h[n - 1];
#endif
    for (i = 0; i < n; i++)
        out[i] = t[i];
    return ED_OK;
}

int ed_compatible(struct ed_interface a, struct ed_interface b, unsigned *out)
{
    if (!out || !a.protocol || !b.protocol || !a.version || !b.version ||
        a.shape > 1 || b.shape > 1 || a.ownership > 1 || b.ownership > 1)
        return ED_INVALID;
    *out = a.protocol == b.protocol && a.version == b.version &&
           a.shape == b.shape && a.ownership == b.ownership;
#ifdef ED_MUTATE_LAYERS2
    *out = a.protocol == b.protocol;
#endif
    return ED_OK;
}

static uint32_t crc(const uint8_t *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    size_t i;
    unsigned j;
    for (i = 0; i < n; i++)
    {
        c ^= p[i];
        for (j = 0; j < 8; j++)
            c = (c >> 1) ^ ((c & 1u) ? UINT32_C(0xedb88320) : 0);
    }
    return c ^ UINT32_MAX;
}

int ed_crc32(const uint8_t *p, size_t n, uint32_t *out)
{
    if (!out || (!p && n) || n > 1518)
        return ED_INVALID;
    *out = crc(p, n);
    return ED_OK;
}

static int field_ok(uint16_t f)
{
    return !(f >= 1501 && f <= 1535) && f != 0x8100;
}

int ed_frame_build(const uint8_t da[6], const uint8_t sa[6], uint16_t field,
                   const uint8_t *payload, size_t n, unsigned fcs, uint8_t *out,
                   size_t cap, size_t *written)
{
    uint8_t temp[1518];
    size_t i, total, stored;
    uint32_t c;
    if (!da || !sa || (!payload && n) || !out || !written || n > 1500 ||
        fcs > 1 || !field_ok(field) || (field <= 1500 && field != n))
        return ED_INVALID;
    stored = n < 46 ? 46 : n;
    total = 14 + stored + 4 * fcs;
    if (cap < total)
        return ED_FULL;
    for (i = 0; i < 6; i++)
    {
        temp[i] = da[i];
        temp[i + 6] = sa[i];
    }
    temp[12] = (uint8_t)(field >> 8);
    temp[13] = (uint8_t)field;
    for (i = 0; i < stored; i++)
        temp[14 + i] = i < n ? payload[i] : 0;
#ifdef ED_MUTATE_FRAME
    if (stored > n)
        temp[14 + n] = 1;
#endif
    if (fcs)
    {
        c = crc(temp, 14 + stored);
        for (i = 0; i < 4; i++)
            temp[14 + stored + i] = (uint8_t)(c >> (8 * i));
    }
    for (i = 0; i < total; i++)
        out[i] = temp[i];
    *written = total;
    return ED_OK;
}

int ed_frame_parse(const uint8_t *p, size_t n, unsigned fcs,
                   struct ed_frame *out)
{
    struct ed_frame r;
    size_t i, data;
    uint32_t expected, actual = 0;
    if (!p || !out || fcs > 1 || n < 60 + 4 * fcs || n > 1514 + 4 * fcs)
        return ED_INVALID;
    r.field = (uint16_t)((unsigned)p[12] * 256u + p[13]);
    if (!field_ok(r.field))
        return ED_UNSUPPORTED;
    data = n - 14 - 4 * fcs;
    if (r.field <= 1500 && r.field > data)
        return ED_INVALID;
    if (fcs)
    {
        expected = crc(p, n - 4);
        for (i = 0; i < 4; i++)
            actual |= (uint32_t)p[n - 4 + i] << (8 * i);
#ifndef ED_MUTATE_FRAME2
        if (actual != expected)
            return ED_IO;
#else
        (void)actual;
        (void)expected;
#endif
    }
    for (i = 0; i < 6; i++)
    {
        r.destination[i] = p[i];
        r.source[i] = p[i + 6];
    }
    r.payload = p + 14;
    r.stored_payload = data;
    r.payload_length = r.field <= 1500 ? r.field : data;
    *out = r;
    return ED_OK;
}

static int same_mac(const uint8_t *a, const uint8_t *b)
{
    unsigned i;
    for (i = 0; i < 6; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int ed_bridge(struct ed_bridge *s, const uint8_t source[6],
              const uint8_t dest[6], unsigned ingress, uint32_t now,
              unsigned *mask)
{
    struct ed_bridge next;
    unsigned i, free_slot = 4, found = 4, output;
    if (!s || !source || !dest || !mask || ingress < 1 || ingress > 3 ||
        (source[0] & 1) || now < s->now)
        return ED_INVALID;
    next = *s;
    for (i = 0; i < 4; i++)
    {
        struct ed_bridge_entry *e = &next.entries[i];
        if (e->live)
        {
            if (e->port < 1 || e->port > 3 || e->seen > s->now)
                return ED_INVALID;
#ifndef ED_MUTATE_BRIDGE
            if (now - e->seen >= 10)
                e->live = 0;
#else
            if (now - e->seen > 10)
                e->live = 0;
#endif
        }
        if (!e->live && free_slot == 4)
            free_slot = i;
        if (e->live && same_mac(e->mac, source))
            found = i;
    }
    if (found == 4)
    {
        if (free_slot == 4)
            return ED_FULL;
        found = free_slot;
    }
    for (i = 0; i < 6; i++)
        next.entries[found].mac[i] = source[i];
    next.entries[found].port = (uint8_t)ingress;
    next.entries[found].seen = now;
    next.entries[found].live = 1;
    next.now = now;
    output = 7u & ~(1u << (ingress - 1));
    if (!(dest[0] & 1))
        for (i = 0; i < 4; i++)
            if (next.entries[i].live && same_mac(next.entries[i].mac, dest))
            {
                output = next.entries[i].port == ingress
                             ? 0
                             : 1u << (next.entries[i].port - 1);
                break;
            }
#ifdef ED_MUTATE_BRIDGE2
    if (output == 0)
        output = 7u & ~(1u << (ingress - 1));
#endif
    *s = next;
    *mask = output;
    return ED_OK;
}

int ed_mdio_word(uint32_t hz, unsigned phy, unsigned reg, unsigned busy,
                 uint32_t *word, uint32_t *divisor)
{
    unsigned cr, div;
    if (!word || !divisor || phy > 31 || reg > 31 || busy > 1)
        return ED_INVALID;
    if (busy)
        return ED_BUSY;
    if (hz < 20000000 || hz > 180000000)
        return ED_UNSUPPORTED;
    if (hz < 35000000)
    {
        cr = 2;
        div = 16;
    }
    else if (hz < 60000000)
    {
        cr = 3;
        div = 26;
    }
    else if (hz < 100000000)
    {
        cr = 0;
        div = 42;
    }
    else if (hz < 150000000)
    {
        cr = 1;
        div = 62;
    }
    else
    {
        cr = 4;
        div = 102;
    }
    *word = (phy << 11) | (reg << 6) | (cr << 2) | 1;
    *divisor = div;
    return ED_OK;
}

int ed_mdio_read(const struct ed_bus *b, uint32_t hz, unsigned phy,
                 unsigned reg, unsigned budget, uint32_t timeout,
                 uint16_t *value)
{
    uint32_t status, word, div, start, now, last, data;
    unsigned i;
    int result;
    if (!b || !b->read || !b->write || !b->clock || !value || !budget ||
        budget > 1024 || !timeout)
        return ED_INVALID;
    result = ed_mdio_word(hz, phy, reg, 0, &word, &div);
    if (result)
        return result;
    (void)div;
    if (b->read(b->context, 0, &status))
        return ED_IO;
    if (status & 1)
        return ED_BUSY;
    start = last = b->clock(b->context);
    if (b->write(b->context, 0, word & ~1u) || b->write(b->context, 0, word))
        return ED_IO;
    for (i = 0; i < budget; i++)
    {
        now = b->clock(b->context);
        if (now < last || now - start >= timeout)
            return ED_TIMEOUT;
        last = now;
        if (b->read(b->context, 0, &status))
            return ED_IO;
        if (!(status & 1))
        {
            if (b->read(b->context, 1, &data))
                return ED_IO;
            *value = (uint16_t)data;
            return ED_OK;
        }
    }
    return ED_TIMEOUT;
}

int ed_phy_query(const struct ed_bus *b, uint32_t hz, unsigned phy,
                 unsigned budget, uint32_t timeout, struct ed_phy *out)
{
    uint16_t id1, id2, old, current, special;
    struct ed_phy r = {0};
    int e;
    unsigned mode;
    if (!out)
        return ED_INVALID;
    if ((e = ed_mdio_read(b, hz, phy, 2, budget, timeout, &id1)) ||
        (e = ed_mdio_read(b, hz, phy, 3, budget, timeout, &id2)))
        return e;
    if (id1 != 7 || (id2 & 0xfff0u) != 0xc130u)
        return ED_WRONG_PHY;
    if ((e = ed_mdio_read(b, hz, phy, 1, budget, timeout, &old)) ||
        (e = ed_mdio_read(b, hz, phy, 1, budget, timeout, &current)) ||
        (e = ed_mdio_read(b, hz, phy, 31, budget, timeout, &special)))
        return e;
#ifdef ED_MUTATE_PHY
    current = old;
#endif
    r.historical = (uint8_t)((old & 4) != 0);
    r.current = (uint8_t)((current & 4) != 0);
    if (!r.current)
        return ED_LINK_DOWN;
    if (!(current & 32) || !(special & 4096))
        return ED_NEGOTIATING;
    mode = (special >> 2) & 7;
    if (mode == 1 || mode == 2)
        return ED_UNSUPPORTED;
    if (mode != 5 && mode != 6)
        return ED_UNSUPPORTED;
#ifdef ED_MUTATE_PHY2
    mode = 5;
#endif
    r.full = 1;
    r.speed = (uint16_t)(mode == 6 ? 100 : 10);
    *out = r;
    return ED_OK;
}

int ed_rx_acquire(struct ed_rx *s, const struct ed_descriptor *d, size_t n,
                  struct ed_borrow *out)
{
    struct ed_borrow r = {0};
    size_t i, total = 0, exposed;
    uint32_t flags, last;
    if (!s || !d || !out || n < 1 || n > 2)
        return ED_INVALID;
    if (s->held)
        return ED_BUSY;
    for (i = 0; i < n; i++)
    {
        if (!d[i].bytes || !d[i].capacity || d[i].capacity > 1536)
            return ED_INVALID;
        total += d[i].capacity;
        if (d[i].status & UINT32_C(0x80000000))
            return ED_BUSY;
    }
    if (total > 1536)
        return ED_INVALID;
    flags = d[0].status;
    last = d[n - 1].status;
    (void)flags;
#ifndef ED_MUTATE_RX
    if (!(flags & (1u << 9)))
    {
        s->dropping = 1;
        return ED_IO;
    }
#endif
    s->dropping = 0;
    if (!(last & (1u << 8)))
    {
        s->dropping = 1;
        return ED_BUSY;
    }
    for (i = 0; i < n; i++)
        if ((d[i].status & ((1u << 15) | (1u << 14))) ||
            (i && d[i].status & (1u << 9)) ||
            (i + 1 < n && d[i].status & (1u << 8)))
        {
            s->dropping = 1;
            return ED_IO;
        }
    exposed = (last >> 16) & 0x3fffu;
    if (exposed < 64 || exposed > 1518 || exposed > total ||
        (n == 2 && exposed <= d[0].capacity))
    {
        s->dropping = 1;
        return ED_IO;
    }
#ifndef ED_MUTATE_RX2
    exposed -= 4;
#endif
    if (s->generation == UINT32_MAX)
        return ED_FULL;
    r.first = d[0].bytes;
    r.first_length =
        (uint16_t)(exposed < d[0].capacity ? exposed : d[0].capacity);
    if (exposed > r.first_length)
    {
        r.second = d[1].bytes;
        r.second_length = (uint16_t)(exposed - r.first_length);
    }
    r.token = ++s->generation;
    s->held = 1;
    *out = r;
    return ED_OK;
}

int ed_rx_release(struct ed_rx *s, uint32_t token)
{
    if (!s)
        return ED_INVALID;
    if (!s->held || !token || token != s->generation)
        return ED_STALE;
    s->held = 0;
    return ED_OK;
}

static int tx_valid(struct ed_tx *s, unsigned slot, uint32_t token)
{
    if (!s || slot > 1 || !token)
        return ED_INVALID;
    if (s->slots[slot].token != token || !s->slots[slot].phase)
        return ED_STALE;
    return ED_OK;
}

int ed_tx_prepare(struct ed_tx *s, unsigned slot, uint32_t token,
                  const uint8_t *p, size_t n)
{
    size_t i;
    struct ed_tx_slot *t;
    if (!s || slot > 1 || !p || n < 14 || n > 1514 || !token)
        return ED_INVALID;
    t = &s->slots[slot];
    if (t->phase)
        return ED_BUSY;
    if (token <= t->token)
        return ED_STALE;
    for (i = 0; i < n; i++)
        t->bytes[i] = p[i];
    t->length = (uint16_t)n;
    t->token = token;
    t->head = 1u << 28;
    t->tail = 1u << 29;
    t->phase = 1;
    return ED_OK;
}

int ed_tx_tail(struct ed_tx *s, unsigned slot, uint32_t token)
{
    int e = tx_valid(s, slot, token);
    struct ed_tx_slot *t;
    if (e)
        return e;
    t = &s->slots[slot];
    if (t->phase != 1)
        return ED_BUSY;
    t->tail |= UINT32_C(0x80000000);
    t->phase = 2;
    return ED_OK;
}

int ed_tx_publish(struct ed_tx *s, unsigned slot, uint32_t token,
                  void (*barrier)(void *), void *context)
{
    int e = tx_valid(s, slot, token);
    struct ed_tx_slot *t;
    if (e)
        return e;
    if (!barrier)
        return ED_INVALID;
    t = &s->slots[slot];
#ifndef ED_MUTATE_TX
    if (t->phase != 2)
        return ED_BUSY;
#else
    if (t->phase != 1 && t->phase != 2)
        return ED_BUSY;
#endif
    if (s->slots[1 - slot].phase >= 3)
        return ED_BUSY;
    barrier(context);
    t->head |= UINT32_C(0x80000000);
    t->phase = 3;
    return ED_OK;
}

int ed_tx_complete(struct ed_tx *s, unsigned slot, uint32_t token,
                   unsigned returned, unsigned error)
{
    int e = tx_valid(s, slot, token);
    struct ed_tx_slot *t;
    if (e)
        return e;
    if (returned > 1 || error > 1)
        return ED_INVALID;
    t = &s->slots[slot];
    if (t->phase != 3)
        return ED_STALE;
    if (!returned)
        return ED_BUSY;
    t->phase = 0;
    t->head = t->tail = 0;
    return error ? ED_IO : ED_OK;
}

int ed_tx_abort(struct ed_tx *s, unsigned slot, uint32_t token,
                unsigned stopped)
{
    int e = tx_valid(s, slot, token);
    struct ed_tx_slot *t;
    if (e)
        return e;
    if (stopped > 1)
        return ED_INVALID;
    t = &s->slots[slot];
#ifndef ED_MUTATE_TX2
    if (!stopped)
    {
        t->phase = 4;
        return ED_BUSY;
    }
#else
    (void)stopped;
#endif
    t->phase = 0;
    t->head = t->tail = 0;
    return ED_OK;
}

int ed_reachable(uint32_t address, uint32_t n)
{
    return address >= UINT32_C(0x20000000) && address < UINT32_C(0x20030000) &&
                   !(address & 3u) && n >= 1 && n <= 1536 &&
                   n <= UINT32_C(0x20030000) - address
               ? ED_OK
               : ED_INVALID;
}

int ed_initialize(const uint8_t *events, size_t n, size_t *bad)
{
    /* Dependencies are this exercise's conservative board-adapter contract. */
    static const unsigned need[10] = {0, 1, 1, 7, 15, 31, 15, 591, 767, 15};
    unsigned seen = 0, event;
    size_t i;
    if (!bad || (!events && n) || n > 10)
        return ED_INVALID;
    for (i = 0; i < n; i++)
    {
        event = events[i];
        if (event > 9 || (seen & (1u << event)) ||
            (seen & need[event]) != need[event])
        {
            *bad = i;
            return ED_INVALID;
        }
        seen |= 1u << event;
    }
    *bad = n;
    return seen == 1023 ? ED_OK : ED_INVALID;
}

int ed_budget(uint32_t app, unsigned mb, struct ed_budget *out)
{
    struct ed_budget b;
    uint32_t data;
    if (!out || app > 1460 || (mb != 10 && mb != 100))
        return ED_INVALID;
    b.ip = 40 + app;
    data = b.ip < 46 ? 46 : b.ip;
#ifdef ED_MUTATE_HANDOFF
    data = b.ip;
#endif
    b.frame = 14 + data + 4;
    b.slot = b.frame + 20;
    b.nanoseconds = b.slot * (mb == 100 ? 80u : 800u);
    *out = b;
    return ED_OK;
}

int ed_capture(const uint32_t *times, size_t n, int32_t offset, unsigned known,
               uint32_t uncertainty, uint64_t *lower, uint64_t *upper)
{
    uint64_t lo[16], hi[16];
    int64_t center;
    size_t i;
    if (n > 16 || (!times && n) || (!lower && n) || (!upper && n) || known > 1)
        return ED_INVALID;
    if (!known)
        return ED_UNSUPPORTED;
    for (i = 0; i < n; i++)
    {
        if (i && times[i] < times[i - 1])
            return ED_INVALID;
        center = (int64_t)times[i] + offset;
        if (center < (int64_t)uncertainty)
            return ED_INVALID;
        lo[i] = (uint64_t)center - uncertainty;
        hi[i] = (uint64_t)center + uncertainty;
#ifdef ED_MUTATE_HANDOFF2
        lo[i] = hi[i] = (uint64_t)center;
#endif
    }
    for (i = 0; i < n; i++)
    {
        lower[i] = lo[i];
        upper[i] = hi[i];
    }
    return ED_OK;
}

int ed_first_failure(unsigned mask, unsigned *out)
{
    unsigned i;
    if (!out || mask > 63)
        return ED_INVALID;
    for (i = 0; i < 6; i++)
        if (!(mask & (1u << i)))
            break;
    *out = i;
    return ED_OK;
}
