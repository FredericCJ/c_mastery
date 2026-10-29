#include "arp.h"
#ifndef NA_MUTANT
#define NA_MUTANT 0
#endif
static void copy(unsigned char *to, const unsigned char *from, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        to[i] = from[i];
}

static int equal(const unsigned char *a, const unsigned char *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        if (a[i] != b[i])
            return 0;
    return 1;
}

static unsigned word(const unsigned char *p)
{
    return (unsigned)p[0] * 256 + p[1];
}

static void store(unsigned char *p, unsigned value)
{
    p[0] = (unsigned char)(value >> 8);
    p[1] = (unsigned char)value;
}

static int unicast(const unsigned char *mac)
{
    unsigned nonzero = 0;
    for (unsigned i = 0; i < 6; ++i)
        nonzero |= mac[i];
    return nonzero && !(mac[0] & 1);
}

static int valid(const struct na_arp *s, uint64_t now)
{
    return s && s->epoch && now >= s->last;
}

int na_init(struct na_arp *s, struct na_config c, uint32_t epoch)
{
    if (!s || !epoch || !unicast(c.local_mac) ||
        equal(c.local_ip, c.peer_ip, 4))
        return NA_INVALID;
    *s = (struct na_arp){.config = c, .epoch = epoch};
    return NA_OK;
}

int na_queue(struct na_arp *s, const void *p, size_t n, uint64_t now)
{
    if (!valid(s, now) || !p || !n || n > 1500)
        return NA_INVALID;
    if (s->pending || s->prepared)
        return NA_BUSY;
    if (now > UINT64_MAX - 3000)
        return NA_RANGE;
    copy(s->datagram, p, n);
    s->length = (uint16_t)n;
    s->pending = 1;
    s->started = now;
    s->deadline = now + 3000;
    s->slots_done = s->requests = s->resolved = 0;
    s->last = now;
    return NA_OK;
}

int na_receive(struct na_arp *s, const void *data, size_t n, uint64_t now)
{
    static const unsigned char broadcast[6] = {255, 255, 255, 255, 255, 255};
    if (!valid(s, now) || !data || n < 42 || n > 1514)
        return NA_INVALID;
    if (s->prepared || s->reply_pending)
        return NA_BUSY;
    if (now > UINT64_MAX - 30000)
        return NA_RANGE;
    const unsigned char *p = data, *a = p + 14;
    if ((!equal(p, s->config.local_mac, 6) && !equal(p, broadcast, 6)) ||
        !unicast(p + 6) || word(p + 12) != 0x806 || word(a) != 1 ||
        word(a + 2) != 0x800 || a[4] != 6 || a[5] != 4 ||
        (word(a + 6) != 1 && word(a + 6) != 2) ||
        (NA_MUTANT != 1 && !equal(p + 6, a + 8, 6)) ||
        (NA_MUTANT != 2 && !equal(a + 14, s->config.peer_ip, 4)) ||
        !equal(a + 24, s->config.local_ip, 4) ||
        (word(a + 6) == 2 && !equal(a + 18, s->config.local_mac, 6)))
        return NA_INVALID;
    copy(s->peer_mac, a + 8, 6);
    s->known = 1;
    s->expires = now + 30000;
    s->last = now;
    if (word(a + 6) == 1)
    {
        copy(s->reply_mac, a + 8, 6);
        s->reply_pending = 1;
    }
    return NA_OK;
}

static void frame(struct na_arp *s, struct net_packet *p, unsigned kind)
{
    for (size_t i = 0; i < sizeof p->bytes; ++i)
        p->bytes[i] = 0;
    const unsigned char fixture_mac[6] = {2, 0, 0, 0, 0, 1};
    const unsigned char *destination =
        kind == NA_REPLY ? s->reply_mac : s->peer_mac;
    if (kind == NA_REQUEST)
        for (unsigned i = 0; i < 6; ++i)
            p->bytes[i] = 255;
    else
        copy(p->bytes,
             NA_MUTANT == 3 && kind == NA_DATA ? fixture_mac : destination, 6);
    copy(p->bytes + 6, s->config.local_mac, 6);
    store(p->bytes + 12, kind == NA_DATA ? 0x800 : 0x806);
    if (kind == NA_DATA)
    {
        copy(p->bytes + 14, s->datagram, s->length);
        p->length = 14 + s->length;
    }
    else
    {
        unsigned char *a = p->bytes + 14;
        store(a, 1);
        store(a + 2, 0x800);
        a[4] = 6;
        a[5] = 4;
        store(a + 6, kind == NA_REPLY ? 2 : 1);
        copy(a + 8, s->config.local_mac, 6);
        copy(a + 14, s->config.local_ip, 4);
        if (kind == NA_REPLY)
            copy(a + 18, destination, 6);
        copy(a + 24, s->config.peer_ip, 4);
        p->length = 42;
    }
    if (p->length < 60)
        p->length = 60;
}

int na_prepare(struct na_arp *s, uint64_t now, struct net_packet *out,
               struct na_ticket *ticket)
{
    if (!valid(s, now) || !out || !ticket)
        return NA_INVALID;
    if (s->prepared)
        return NA_BUSY;
    s->last = now;
    if (s->known && (NA_MUTANT != 4 && now >= s->expires))
        s->known = 0;
    if (s->pending && now >= s->deadline)
    {
        s->pending = 0;
        s->length = 0;
        return NA_EXPIRED;
    }
    unsigned kind = 0, slot = 0;
    if (s->reply_pending)
        kind = NA_REPLY;
    else if (s->pending && s->known)
        kind = NA_DATA;
    else if (s->pending)
    {
        /* Pending deadline established a difference below3000. */
        unsigned elapsed = (unsigned)(now - s->started);
        slot = elapsed / (NA_MUTANT == 5 ? 100 : 1000);
        if (slot > 2 || (s->slots_done & (1u << slot)))
            return NA_WAIT;
        kind = NA_REQUEST;
    }
    else
        return NA_WAIT;
    if (s->serial == UINT32_MAX)
        return NA_RANGE;
    frame(s, out, kind);
    ++s->serial;
    s->prepared = kind;
    s->prepared_slot = slot;
    *ticket = (struct na_ticket){s->epoch, s->serial, kind};
    return NA_OK;
}

int na_submit(struct na_arp *s, struct na_ticket t, unsigned accepted)
{
    if (!s || accepted > 1)
        return NA_INVALID;
    if (!s->prepared || t.epoch != s->epoch || t.serial != s->serial ||
        t.kind != s->prepared)
        return NA_STALE;
    if (accepted || NA_MUTANT == 6)
    {
        if (s->prepared == NA_REQUEST)
        {
            s->slots_done |= 1u << s->prepared_slot;
            ++s->requests;
        }
        else if (s->prepared == NA_DATA)
        {
            s->pending = 0;
            s->length = 0;
            s->resolved = 1;
        }
        else
            s->reply_pending = 0;
    }
    s->prepared = 0;
    return NA_OK;
}
