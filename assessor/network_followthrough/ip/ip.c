#include "ip.h"
#ifndef NI_MUTANT
#define NI_MUTANT 0
#endif
static unsigned u16(const unsigned char *p)
{
    return (unsigned)p[0] * 256u + p[1];
}

static uint32_t u32(const unsigned char *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
           p[3];
}

static void put16(unsigned char *p, unsigned x)
{
    p[0] = (unsigned char)(x >> 8);
    p[1] = (unsigned char)x;
}

static void put32(unsigned char *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = (unsigned char)(x >> (24 - 8 * i));
    }
}

static uint16_t checksum(const unsigned char *p, size_t n)
{
    struct s_checksum s = {0};
    uint16_t out = 0;
    (void)s_checksum_add(&s, p, n);
    (void)s_checksum_finish(&s, &out);
    return out;
}

static int config_ok(const struct ni_config *c)
{
    return c && c->local && c->peer && c->local != c->peer;
}

int ni_parse(const struct ni_config *c, const unsigned char *p, size_t n,
             unsigned fragments, struct ni_view *v)
{
    if (!config_ok(c) || !p || !v || fragments > 1 || n < 20 || n > 1500)
    {
        return NI_INVALID;
    }
    unsigned total = u16(p + 2), flags = u16(p + 6),
             offset = (flags & 8191u) * 8u, more = (flags >> 13) & 1u;
    if (p[0] != 0x45 || total < 20 || total > 1500 ||
        (NI_MUTANT != 1 && total > n) || !p[8] || (p[9] != 1 && p[9] != 6) ||
        u32(p + 12) != c->peer || u32(p + 16) != c->local ||
        (NI_MUTANT != 2 && checksum(p, 20)) || (flags & 0x8000u) ||
        ((flags & 0x4000u) && (more || offset)) ||
        (!fragments && (more || offset)))
    {
        return NI_INVALID;
    }
    unsigned len = total - 20;
    if ((more || offset) && (!len || (more && (len & 7u))))
    {
        return NI_INVALID;
    }
    if (offset > 1480 || len > 1480 - offset)
    {
        return NI_INVALID;
    }
    struct ni_view out = {p,          p + 20, len,  u32(p + 12), u32(p + 16),
                          u16(p + 4), offset, more, p[9]};
    *v = out;
    return NI_OK;
}

int ni_echo(const struct ni_config *c, const unsigned char *p, size_t n,
            struct ni_packet *out)
{
    struct ni_view v;
    if (!out || ni_parse(c, p, n, 0, &v) != NI_OK || v.protocol != 1 ||
        v.length < 8 || v.length > 72 || v.payload[0] != 8 || v.payload[1] ||
        checksum(v.payload, v.length))
    {
        return NI_INVALID;
    }
    /* Admission is complete before any output write. No temporary 1500-byte
     * object. */
    for (unsigned i = 0; i < 20; i++)
    {
        out->bytes[i] = 0;
    }
    out->bytes[0] = 0x45;
    put16(out->bytes + 2, (unsigned)v.length + 20);
    put16(out->bytes + 6, 0x4000);
    out->bytes[8] = 64;
    out->bytes[9] = 1;
    put32(out->bytes + 12, c->local);
    put32(out->bytes + 16, c->peer);
    for (size_t i = 0; i < v.length; i++)
    {
        out->bytes[20 + i] = v.payload[i];
    }
    out->bytes[20] = 0;
    out->bytes[22] = out->bytes[23] = 0;
    if (NI_MUTANT == 3)
    {
        out->bytes[24] = 0;
    }
    put16(out->bytes + 22, checksum(out->bytes + 20, v.length));
    put16(out->bytes + 10, checksum(out->bytes, 20));
    out->length = v.length + 20;
    return NI_OK;
}

int ni_init(struct ni_reassembly *s, struct ni_config c, unsigned slots,
            unsigned policy, uint64_t now, uint64_t timeout)
{
    if (!s || !config_ok(&c) || slots < 1 || slots > 2 ||
        policy > NI_IDENTICAL || !timeout || timeout > 120000 ||
        now > UINT64_MAX - timeout)
    {
        return NI_INVALID;
    }
    s->config = c;
    s->last = now;
    s->timeout = timeout;
    s->slots = slots;
    s->policy = policy;
    for (unsigned i = 0; i < 2; i++)
    {
        s->slot[i].state = NI_FREE;
    }
    return NI_OK;
}

int ni_tick(struct ni_reassembly *s, uint64_t now)
{
    if (!s || now < s->last)
    {
        return NI_INVALID;
    }
    for (unsigned i = 0; i < s->slots; i++)
    {
        if ((s->slot[i].state == NI_ACTIVE || s->slot[i].state == NI_POISON) &&
            now >= s->slot[i].expiry)
        {
            s->slot[i].state = NI_FREE;
        }
    }
    s->last = now;
    return NI_OK;
}

int ni_expire(struct ni_reassembly *s, uint64_t now, struct ni_packet out[2],
              unsigned *count)
{
    if (!s || !out || !count || now < s->last)
    {
        return NI_INVALID;
    }
    unsigned emitted = 0;
    for (unsigned i = 0; i < s->slots; i++)
    {
        const struct ni_slot *a = &s->slot[i];
        if (NI_MUTANT == 8 || a->state != NI_ACTIVE || now < a->expiry ||
            !a->first ||
            (a->protocol == 1 && a->bytes[0] != 0 && a->bytes[0] != 8))
        {
            continue;
        }
        struct ni_packet *p = &out[emitted++];
        for (unsigned j = 0; j < 28; j++)
        {
            p->bytes[j] = 0;
        }
        p->bytes[0] = 0x45;
        put16(p->bytes + 2, 56);
        p->bytes[8] = 64;
        p->bytes[9] = 1;
        put32(p->bytes + 12, s->config.local);
        put32(p->bytes + 16, a->source);
        p->bytes[20] = 11;
        p->bytes[21] = 1;
        for (unsigned j = 0; j < 20; j++)
        {
            p->bytes[28 + j] = a->header[j];
        }
        for (unsigned j = 0; j < 8; j++)
        {
            p->bytes[48 + j] = a->bytes[j];
        }
        put16(p->bytes + 22, checksum(p->bytes + 20, 36));
        put16(p->bytes + 10, checksum(p->bytes, 20));
        p->length = 56;
    }
    (void)ni_tick(s, now);
    *count = emitted;
    return NI_OK;
}

static unsigned has(const struct ni_slot *s, unsigned i)
{
    return (s->bitmap[i / 8] >> (i % 8)) & 1u;
}

static int poison(struct ni_slot *s)
{
    s->state = NI_POISON;
    return NI_CONFLICT;
}

int ni_receive(struct ni_reassembly *s, const unsigned char *p, size_t n,
               uint64_t now, unsigned *index)
{
    struct ni_view v;
    if (!s || !index || ni_parse(&s->config, p, n, 1, &v) != NI_OK ||
        now < s->last || now > UINT64_MAX - s->timeout)
    {
        return NI_INVALID;
    }
    (void)ni_tick(s, now);
    unsigned chosen = 2, empty = 2;
    for (unsigned i = 0; i < s->slots; i++)
    {
        struct ni_slot *a = &s->slot[i];
        if (a->state == NI_FREE)
        {
            if (empty == 2)
            {
                empty = i;
            }
            continue;
        }
        if (a->id == v.id && a->source == v.source &&
            a->destination == v.destination &&
            (NI_MUTANT == 4 || a->protocol == v.protocol))
        {
            chosen = i;
            break;
        }
    }
    if (chosen == 2)
    {
        if (empty == 2)
        {
            return NI_BUSY;
        }
        chosen = empty;
        struct ni_slot *a = &s->slot[chosen];
        a->source = v.source;
        a->destination = v.destination;
        a->id = v.id;
        a->protocol = v.protocol;
        a->state = NI_ACTIVE;
        a->expiry = now + s->timeout;
        a->total = 0;
        a->have_last = 0;
        a->first = 0;
        a->covered = 0;
        a->high = 0;
        a->nonfinal = 0;
        for (unsigned j = 0; j < 185; j++)
        {
            a->bitmap[j] = 0;
        }
    }
    struct ni_slot *a = &s->slot[chosen];
    if (a->state == NI_POISON)
    {
        return NI_CONFLICT;
    }
    if (a->state == NI_READY)
    {
        return NI_BUSY;
    }
    unsigned end = v.offset + (unsigned)v.length;
    if ((a->have_last && (end > a->total || (!v.more && end != a->total) ||
                          (v.more && end >= a->total))) ||
        (!v.more && (end < a->high || end <= a->nonfinal)))
    {
        return poison(a);
    }
    /* Validate the entire overlap before publishing any new octet. */
    for (unsigned j = 0; j < v.length; j++)
    {
        if (has(a, v.offset + j) && NI_MUTANT != 5 &&
            (s->policy == NI_STRICT || a->bytes[v.offset + j] != v.payload[j]))
        {
            return poison(a);
        }
    }
    if (NI_MUTANT == 6)
    {
        a->expiry = now + s->timeout;
    }
    for (unsigned j = 0; j < v.length; j++)
    {
        unsigned k = v.offset + j;
        if (!has(a, k))
        {
            a->covered++;
        }
        a->bytes[k] = v.payload[j];
        a->bitmap[k / 8] |= (unsigned char)(1u << (k % 8));
    }
    if (end > a->high)
    {
        a->high = end;
    }
    if (v.more)
    {
        if (end > a->nonfinal)
        {
            a->nonfinal = end;
        }
    }
    else
    {
        a->total = end;
        a->have_last = 1;
    }
    if (!v.offset && !a->first)
    {
        for (unsigned j = 0; j < 20; j++)
        {
            a->header[j] = v.header[j];
        }
        a->first = 1;
    }
    if (a->have_last && a->first && (NI_MUTANT == 7 || a->covered == a->total))
    {
        a->state = NI_READY;
    }
    *index = chosen;
    return a->state == NI_READY ? NI_OK : NI_WAIT;
}

int ni_take(struct ni_reassembly *s, unsigned index, struct ni_packet *out)
{
    if (!s || !out || index >= s->slots)
    {
        return NI_INVALID;
    }
    struct ni_slot *a = &s->slot[index];
    if (a->state != NI_READY)
    {
        return NI_WAIT;
    }
    for (unsigned j = 0; j < 20; j++)
    {
        out->bytes[j] = a->header[j];
    }
    for (unsigned j = 0; j < a->total; j++)
    {
        out->bytes[20 + j] = a->bytes[j];
    }
    put16(out->bytes + 2, a->total + 20);
    put16(out->bytes + 6, 0);
    put16(out->bytes + 10, 0);
    put16(out->bytes + 10, checksum(out->bytes, 20));
    out->length = a->total + 20;
    /* Keep a tombstone until the original deadline: delayed duplicate fragments
     * must not recreate and redeliver the same identity during this window. */
    a->state = NI_POISON;
    return NI_OK;
}
