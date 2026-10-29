#include "wire_depth.h"
#include <string.h>
#ifndef RPW_MUTATION
#define RPW_MUTATION 0
#endif
static int limits(struct rpw_limits l)
{
    return l.depth >= 1 && l.depth <= 4 && l.bytes >= 4 && l.bytes <= 256 &&
           l.fields >= 2 && l.fields <= 32;
}

static int var(const unsigned char *p, size_t n, size_t *at, uint32_t *out)
{
    uint32_t x = 0;
    unsigned i;
    for (i = 0; i < 5; ++i)
    {
        unsigned b;
        if (*at == n)
        {
            return RPW_INVALID;
        }
        b = p[(*at)++];
        if (i == 4 && b > 15)
        {
            return RPW_INVALID;
        }
        x |= (uint32_t)(b & 127u) << (7 * i);
        if (!(b & 128u))
        {
            if (i && b == 0 && RPW_MUTATION != 1)
            {
                return RPW_INVALID;
            }
            *out = x;
            return RPW_OK;
        }
    }
    return RPW_INVALID;
}

static unsigned put(uint32_t x, unsigned char *p)
{
    unsigned n = 0;
    do
    {
        unsigned b = x & 127u;
        x >>= 7;
        p[n++] = (unsigned char)(b | (x ? 128u : 0));
    } while (x);
    return n;
}

static int field_kind(unsigned tag, unsigned wire, unsigned *kind)
{
    if (tag < 1 || tag > 15)
    {
        return tag >= 16 ? RPW_REQUIRED : RPW_INVALID;
    }
    if (tag == 3 || tag == 4)
    {
        if (wire != 2)
        {
            return RPW_INVALID;
        }
        *kind = 2;
    }
    else if (tag == 1 || tag == 2 || tag == 5 || tag == 6)
    {
        if (wire)
        {
            return RPW_INVALID;
        }
        *kind = 0;
    }
    else if (tag == 7)
    {
        if (wire != 2)
        {
            return RPW_INVALID;
        }
        *kind = 1;
    }
    else
    {
        if (wire != 0 && wire != 2)
        {
            return RPW_INVALID;
        }
        *kind = wire ? 1 : 0;
    }
    return RPW_OK;
}

static int parse(const unsigned char *p, size_t n, struct rpw_limits l,
                 unsigned depth, struct rpw_doc *d, unsigned index,
                 unsigned *fields)
{
    size_t at = 0;
    unsigned seen = 0;
    if (depth > l.depth && RPW_MUTATION != 3)
    {
        return RPW_LIMIT;
    }
    while (at < n)
    {
        uint32_t key, x;
        unsigned kind, tag;
        struct rpw_field f = {0};
        int r = var(p, n, &at, &key);
        if (r)
        {
            return r;
        }
        tag = key >> 3;
        r = field_kind(tag, key & 7u, &kind);
        if (r)
        {
            return r;
        }
        if ((seen & (1u << tag)) && RPW_MUTATION != 2)
        {
            return RPW_INVALID;
        }
        if (d->records[index].count == 8 || *fields == l.fields)
        {
            return RPW_LIMIT;
        }
        ++*fields;
        seen |= 1u << tag;
        f.tag = tag;
        f.kind = kind;
        r = var(p, n, &at, &x);
        if (r)
        {
            return r;
        }
        if (!kind)
        {
            f.value = x;
            if (tag == 5 && (x & ~l.supported) && RPW_MUTATION != 5)
            {
                return RPW_REQUIRED;
            }
        }
        else
        {
            if (x > n - at)
            {
                return RPW_INVALID;
            }
            if (kind == 2)
            {
                unsigned child;
                if (d->count == 16)
                {
                    return RPW_LIMIT;
                }
                child = d->count++;
                f.value = child;
                r = parse(p + at, x, l, depth + 1, d, child, fields);
                if (r)
                {
                    return r;
                }
            }
            else
            {
                if (x > 32)
                {
                    return RPW_LIMIT;
                }
                f.length = x;
                if (x)
                {
                    memcpy(f.bytes, p + at, x);
                }
            }
            at += x;
        }
        d->records[index].fields[d->records[index].count++] = f;
    }
    return (seen & 6u) == 6u ? RPW_OK : RPW_INVALID;
}

int rpw_decode(const unsigned char *p, size_t n, struct rpw_limits l,
               struct rpw_doc *out)
{
    struct rpw_doc d = {0};
    unsigned fields = 0;
    int r;
    if ((!p && n) || !out || !limits(l))
    {
        return RPW_INVALID;
    }
    if (n > 256 || (n > l.bytes && RPW_MUTATION != 4))
    {
        return RPW_LIMIT;
    }
    d.count = 1;
    r = parse(p, n, l, 1, &d, 0, &fields);
    if (RPW_MUTATION == 6)
    {
        *out = d;
    }
    if (r)
    {
        return r;
    }
    *out = d;
    return RPW_OK;
}

static int emit(const struct rpw_doc *d, unsigned index, struct rpw_limits l,
                unsigned depth, unsigned *seen_nodes, unsigned *fields,
                unsigned char out[256], size_t *length)
{
    unsigned seen = 0, i, tag;
    size_t at = 0;
    if (index >= d->count || (*seen_nodes & (1u << index)))
    {
        return RPW_INVALID;
    }
    if (depth > l.depth)
    {
        return RPW_LIMIT;
    }
    *seen_nodes |= 1u << index;
    if (d->records[index].count > 8)
    {
        return RPW_INVALID;
    }
    for (i = 0; i < d->records[index].count; ++i)
    {
        const struct rpw_field *f = &d->records[index].fields[i];
        unsigned kind;
        int r;
        if (f->kind > 2)
        {
            return RPW_INVALID;
        }
        r = field_kind(f->tag, f->kind ? 2 : 0, &kind);
        if (r)
        {
            return r;
        }
        if (kind != f->kind || (seen & (1u << f->tag)))
        {
            return RPW_INVALID;
        }
        seen |= 1u << f->tag;
        if (*fields == l.fields)
        {
            return RPW_LIMIT;
        }
        ++*fields;
        if (f->tag == 5 && (f->value & ~l.supported))
        {
            return RPW_REQUIRED;
        }
        if (f->kind == 1 && f->length > 32)
        {
            return RPW_LIMIT;
        }
    }
    if ((seen & 6u) != 6u)
    {
        return RPW_INVALID;
    }
    for (tag = 1; tag <= 15; ++tag)
    {
        for (i = 0; i < d->records[index].count; ++i)
        {
            const struct rpw_field *f = &d->records[index].fields[i];
            unsigned char child[256], head[10];
            size_t n = 0;
            unsigned h;
            if (f->tag != tag)
            {
                continue;
            }
            h = put((tag << 3) | (f->kind ? 2u : 0u), head);
            if (!f->kind)
            {
                h += put(f->value, head + h);
            }
            else
            {
                int r;
                if (f->kind == 2)
                {
                    r = emit(d, f->value, l, depth + 1, seen_nodes, fields,
                             child, &n);
                    if (r)
                    {
                        return r;
                    }
                }
                else
                {
                    n = f->length;
                    if (n)
                    {
                        memcpy(child, f->bytes, n);
                    }
                }
                h += put((uint32_t)n, head + h);
            }
            if (h > 256 - at || n > 256 - at - h)
            {
                return RPW_LIMIT;
            }
            memcpy(out + at, head, h);
            at += h;
            if (n)
            {
                memcpy(out + at, child, n);
            }
            at += n;
        }
    }
    *length = at;
    return RPW_OK;
}

int rpw_encode(const struct rpw_doc *d, struct rpw_limits l, unsigned char *out,
               size_t cap, size_t *length)
{
    unsigned char staged[256];
    unsigned seen = 0, fields = 0;
    size_t n;
    int r;
    if (!d || !length || (!out && cap) || !limits(l) || !d->count ||
        d->count > 16)
    {
        return RPW_INVALID;
    }
    r = emit(d, 0, l, 1, &seen, &fields, staged, &n);
    if (r)
    {
        return r;
    }
    if (seen != ((1u << d->count) - 1))
    {
        return RPW_INVALID;
    }
    if (n > l.bytes)
    {
        return RPW_LIMIT;
    }
    if (n > cap)
    {
        return RPW_FULL;
    }
    if (n)
    {
        memcpy(out, staged, n);
    }
    *length = n;
    return RPW_OK;
}

int rpw_update(struct rpw_doc *d, struct rpw_limits l, uint32_t id,
               int32_t delta)
{
    unsigned char bytes[256];
    size_t n;
    struct rpw_doc next;
    unsigned i;
    int r;
    if (!d)
    {
        return RPW_INVALID;
    }
    r = rpw_encode(d, l, bytes, sizeof bytes, &n);
    if (r)
    {
        return r;
    }
    next = *d;
    for (i = 0; i < next.records[0].count; ++i)
    {
        struct rpw_field *f = &next.records[0].fields[i];
        if (f->tag == 1)
        {
            f->value = id;
        }
        if (f->tag == 2)
        {
            f->value = rp_zigzag(delta);
        }
    }
    if (RPW_MUTATION == 8)
    {
        next.records[0].count = 2;
        next.count = 1;
    }
    r = rpw_encode(&next, l, bytes, sizeof bytes, &n);
    if (r)
    {
        return r;
    }
    *d = next;
    return RPW_OK;
}

int rpw_project(const struct rpw_doc *d, struct rpw_limits l,
                struct rp_message *out)
{
    unsigned char bytes[256];
    size_t n;
    struct rp_message m = {0};
    unsigned tag, i;
    int r;
    if (!out)
    {
        return RPW_INVALID;
    }
    r = rpw_encode(d, l, bytes, sizeof bytes, &n);
    if (r)
    {
        return r;
    }
    for (tag = 1; tag <= 2; ++tag)
    {
        for (i = 0; i < d->records[0].count; ++i)
        {
            if (d->records[0].fields[i].tag == tag)
            {
                m.fields[m.count++] =
                    (struct rp_field){tag, d->records[0].fields[i].value};
            }
        }
    }
    *out = m;
    return RPW_OK;
}

uint64_t rpw_binary64(uint64_t bits)
{
    uint64_t magnitude = bits & UINT64_C(0x7fffffffffffffff);
    if (!magnitude && RPW_MUTATION != 9)
    {
        return 0;
    }
    if (magnitude > UINT64_C(0x7ff0000000000000))
    {
        return UINT64_C(0x7ff8000000000000);
    }
    return bits;
}

int rpw_utf8(const unsigned char *p, size_t n)
{
    size_t at = 0;
    if (!p && n)
    {
        return RPW_INVALID;
    }
    while (at < n)
    {
        unsigned b = p[at++], left;
        uint32_t cp, min;
        if (b < 128)
        {
            continue;
        }
        if (b >= 0xc2 && b <= 0xdf)
        {
            left = 1;
            cp = b & 31u;
            min = 0x80;
        }
        else if (b >= 0xe0 && b <= 0xef)
        {
            left = 2;
            cp = b & 15u;
            min = 0x800;
        }
        else if (b >= 0xf0 && b <= 0xf4)
        {
            left = 3;
            cp = b & 7u;
            min = 0x10000;
        }
        else
        {
            return RPW_INVALID;
        }
        if (left > n - at)
        {
            return RPW_INVALID;
        }
        while (left--)
        {
            b = p[at++];
            if ((b & 0xc0) != 0x80)
            {
                return RPW_INVALID;
            }
            cp = (cp << 6) | (b & 63u);
        }
        if ((cp < min && RPW_MUTATION != 10) || cp > 0x10ffff ||
            (cp >= 0xd800 && cp <= 0xdfff))
        {
            return RPW_INVALID;
        }
    }
    return RPW_OK;
}

int rpw_stream_init(struct rpw_stream *s, struct rpw_limits l)
{
    struct rpw_stream n = {0};
    if (!s || !limits(l))
    {
        return RPW_INVALID;
    }
    n.limits = l;
    if (rf_parser_init(&n.parser, 1))
    {
        return RPW_INVALID;
    }
    *s = n;
    return RPW_OK;
}

int rpw_feed(struct rpw_stream *s, const unsigned char *p, size_t n,
             size_t *consumed, struct rpw_doc *out)
{
    size_t at = 0;
    if (!s || (!p && n) || !consumed || !out)
    {
        return RPW_INVALID;
    }
    if (s->phase)
    {
        return RPW_STATE;
    }
    while (at < n)
    {
        struct rf_frame frame;
        size_t used = 0;
        int r = rf_feed(&s->parser, p + at, n - at, &used, &frame);
        at += used;
        if (r < 0)
        {
            s->phase = 2;
            *consumed = at;
            return RPW_INVALID;
        }
        if (r == 1)
        {
            if (!frame.length)
            {
                struct rpw_doc d;
                r = rpw_decode(s->bytes, s->used, s->limits, &d);
                s->phase = r ? 2 : 1;
                *consumed = at;
                if (!r)
                {
                    *out = d;
                }
                return r;
            }
            if (frame.length > s->limits.bytes - s->used)
            {
                s->phase = 2;
                *consumed = at;
                return RPW_LIMIT;
            }
            memcpy(s->bytes + s->used, frame.bytes, frame.length);
            s->used += frame.length;
        }
    }
    *consumed = at;
    return RPW_MORE;
}

int rpw_eof(struct rpw_stream *s)
{
    if (!s)
    {
        return RPW_INVALID;
    }
    if (s->phase == 1)
    {
        return RPW_OK;
    }
    if (s->phase)
    {
        return RPW_STATE;
    }
    if (RPW_MUTATION == 7 && rf_eof(&s->parser) == 0)
    {
        return RPW_OK;
    }
    s->phase = 2;
    return RPW_INVALID;
}

int rpw_cancel(struct rpw_stream *s)
{
    if (!s)
    {
        return RPW_INVALID;
    }
    if (s->phase)
    {
        return RPW_STATE;
    }
    memset(s->bytes, 0, sizeof s->bytes);
    s->used = 0;
    s->phase = 3;
    return RPW_OK;
}

int rpw_frame(const struct rpw_doc *d, struct rpw_limits l, unsigned char *out,
              size_t cap, size_t *length)
{
    unsigned char raw[256], wire[289];
    size_t n, at = 0, pos = 0;
    int r;
    if (!length || (!out && cap))
    {
        return RPW_INVALID;
    }
    r = rpw_encode(d, l, raw, sizeof raw, &n);
    if (r)
    {
        return r;
    }
    while (at < n)
    {
        struct rf_frame f = {0};
        struct rf_message m;
        unsigned take = n - at > 8 ? 8 : (unsigned)(n - at);
        f.length = take;
        memcpy(f.bytes, raw + at, take);
        if (rf_message_init(&m, 1, 1, &f))
        {
            return RPW_INVALID;
        }
        memcpy(wire + pos, m.wire, m.wire_length);
        pos += m.wire_length;
        at += take;
    }
    wire[pos++] = 0;
    if (pos > cap)
    {
        return RPW_FULL;
    }
    memcpy(out, wire, pos);
    *length = pos;
    return RPW_OK;
}
