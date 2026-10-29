#include "com_edu7.h"
#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((unsigned)p[0] * 256 + p[1]);
}

static void put16(uint8_t *p, uint16_t n)
{
    p[0] = (uint8_t)(n >> 8);
    p[1] = (uint8_t)n;
}

uint16_t edu_crc(const void *v, size_t n)
{
    const uint8_t *p = v;
    uint16_t c = 0xffff;
    for (size_t i = 0; i < n; i++)
    {
        c ^= (uint16_t)p[i] << 8;
        for (unsigned b = 0; b < 8; b++)
        {
            c = (uint16_t)((c << 1) ^ ((c & 0x8000) ? 0x1021 : 0));
        }
    }
    return c;
}

int edu_frame_encode(const struct edu_frame *f, void *v, size_t cap,
                     size_t *length)
{
    if (f->length > 40 || f->kind > 2 || f->source < 1 || f->source > 2 ||
        f->destination < 1 || f->destination > 2 ||
        f->source == f->destination || (f->kind && f->length))
    {
        return -1;
    }
    uint8_t b[47] = {1, f->kind, f->source, f->destination, f->length};
    memcpy(b + 5, f->payload, f->length);
    size_t n = 7 + f->length;
    put16(b + n - 2, edu_crc(b, n - 2));
    size_t need = 2;
    for (size_t i = 0; i < n; i++)
    {
        need += b[i] == 0x7e || b[i] == 0x7d ? 2 : 1;
    }
    if (need > cap)
    {
        return -1;
    }
    uint8_t *o = v;
    size_t z = 0;
    o[z++] = 0x7e;
    for (size_t i = 0; i < n; i++)
    {
        if (b[i] == 0x7e || b[i] == 0x7d)
        {
            o[z++] = 0x7d;
            o[z++] = b[i] ^ 0x20;
        }
        else
        {
            o[z++] = b[i];
        }
    }
    o[z++] = 0x7e;
    *length = z;
    return 0;
}

int edu_frame_byte(struct edu_framer *p, uint8_t c, int fault,
                   struct edu_frame *out)
{
    if (fault)
    {
        p->discard = 1;
        p->length = 0;
        return -1;
    }
    if (c == 0x7e)
    {
        int r = 0;
        if (p->active && p->length)
        {
            uint8_t *b = p->bytes;
            size_t n = p->length;
            if (p->discard || p->escape || n < 7 || b[0] != 1 || b[1] > 2 ||
                b[2] < 1 || b[2] > 2 || b[3] < 1 || b[3] > 2 || b[2] == b[3] ||
                b[4] > 40 || n != (size_t)b[4] + 7 || (b[1] && b[4]) ||
                edu_crc(b, n - 2) != be16(b + n - 2))
            {
                r = -1;
            }
            else
            {
                struct edu_frame f = {b[1], b[2], b[3], b[4], {0}};
                memcpy(f.payload, b + 5, f.length);
                *out = f;
                r = 1;
            }
        }
        else if (p->discard || p->escape)
        {
            r = -1;
        }
        p->length = p->escape = p->discard = 0;
        p->active = 1;
        return r;
    }
    if (!p->active || p->discard)
    {
        return 0;
    }
    if (p->escape)
    {
        c ^= 0x20;
        p->escape = 0;
    }
    else if (c == 0x7d)
    {
        p->escape = 1;
        return 0;
    }
    if (p->length == sizeof p->bytes)
    {
        p->discard = 1;
        return -1;
    }
    p->bytes[p->length++] = c;
    return 0;
}

int edu_network_encode(const struct edu_datagram *d, void *v, size_t *n)
{
    if (!d->hop || d->length > 34)
    {
        return -1;
    }
    uint8_t *p = v;
    p[0] = 1;
    p[1] = d->source;
    p[2] = d->destination;
    p[3] = d->hop;
    p[4] = 4;
    p[5] = d->length;
    memcpy(p + 6, d->payload, d->length);
    *n = 6 + d->length;
    return 0;
}

int edu_network_decode(const void *v, size_t n, struct edu_datagram *d)
{
    const uint8_t *p = v;
    if (n < 6 || p[0] != 1 || !p[3] || p[4] != 4 || p[5] > 34 ||
        n != (size_t)p[5] + 6)
    {
        return -1;
    }
    struct edu_datagram x = {p[1], p[2], p[3], p[5], {0}};
    memcpy(x.payload, p + 6, x.length);
    *d = x;
    return 0;
}

int edu_route(const struct edu_datagram *in, unsigned ingress,
              struct edu_datagram *out, unsigned *egress, uint8_t *next)
{
    if (in->hop <= 1 || in->source == 34 || ingress > 1)
    {
        return -1;
    }
    unsigned port;
    if (in->destination == 17)
    {
        port = 0;
    }
    else if (in->destination == 51)
    {
        port = 1;
    }
    else
    {
        return -1;
    }
    if (port == ingress)
    {
        return -1;
    }
    *out = *in;
    out->hop--;
    *egress = port;
    *next = port ? 2 : 1;
    return 0;
}

static int fragment_valid(const struct edu_fragment *f)
{
    return f->type >= 1 && f->type <= 3 &&
           ((f->source_port == 7 && f->destination_port == 8) ||
            (f->source_port == 8 && f->destination_port == 7)) &&
           f->id && f->count >= 1 && f->count <= 5 && f->index < f->count &&
           f->credit <= 1 &&
           (f->type == 1 ? (f->length >= 1 && f->length <= 24 &&
                            (f->index + 1 == f->count || f->length == 24))
                         : f->length == 0);
}

int edu_fragment_encode(const struct edu_fragment *f, void *v, size_t *n)
{
    if (!fragment_valid(f))
    {
        return -1;
    }
    uint8_t *p = v;
    p[0] = f->type;
    p[1] = f->source_port;
    p[2] = f->destination_port;
    put16(p + 3, f->id);
    p[5] = f->index;
    p[6] = f->count;
    p[7] = f->credit;
    memcpy(p + 8, f->data, f->length);
    *n = 8 + f->length;
    return 0;
}

int edu_fragment_decode(const void *v, size_t n, struct edu_fragment *f)
{
    const uint8_t *p = v;
    if (n < 8 || n > 32)
    {
        return -1;
    }
    struct edu_fragment x = {p[0], p[1],        p[2], p[5], p[6],
                             p[7], be16(p + 3), {0},  n - 8};
    memcpy(x.data, p + 8, n - 8);
    if (!fragment_valid(&x))
    {
        return -1;
    }
    *f = x;
    return 0;
}

int edu_tx_begin(struct edu_tx *t, uint16_t id, uint8_t src, uint8_t dst,
                 const void *v, size_t n, uint64_t now)
{
    if (t->active || !id || id <= t->id || !n || n > 120 ||
        !((src == 7 && dst == 8) || (src == 8 && dst == 7)))
    {
        return -1;
    }
    memset(t, 0, sizeof *t);
    memcpy(t->bytes, v, n);
    t->length = n;
    t->id = id;
    t->source = src;
    t->destination = dst;
    t->count = (uint8_t)((n + 23) / 24);
    t->credit = 1;
    t->active = 1;
    t->due = now;
    return 0;
}

int edu_tx_poll(struct edu_tx *t, uint64_t now, struct edu_fragment *out)
{
    if (!t->active)
    {
        return 0;
    }
    if (t->blocked && now - t->blocked_since >= 500)
    {
        t->active = 0;
        return -1;
    }
    if (now < t->due)
    {
        return 0;
    }
    if (t->waiting && t->credit && t->retries++ >= 3)
    {
        t->active = 0;
        return -1;
    }
    struct edu_fragment f = {t->credit ? 1 : 3,
                             t->source,
                             t->destination,
                             t->index,
                             t->count,
                             t->credit,
                             t->id,
                             {0},
                             0};
    if (t->credit)
    {
        size_t pos = (size_t)t->index * 24;
        f.length = t->length - pos;
        if (f.length > 24)
        {
            f.length = 24;
        }
        memcpy(f.data, t->bytes + pos, f.length);
        t->waiting = 1;
    }
    t->due = now + 100;
    *out = f;
    return 1;
}

int edu_tx_ack(struct edu_tx *t, const struct edu_fragment *a, uint64_t now)
{
    if (!t->active || !fragment_valid(a) || a->type != 2 ||
        a->source_port != t->destination || a->destination_port != t->source ||
        a->id != t->id || a->index != t->index || a->count != t->count)
    {
        return -1;
    }
    if (t->waiting)
    {
        t->index++;
        t->waiting = 0;
        t->retries = 0;
        if (t->index == t->count)
        {
            t->active = 0;
            return 1;
        }
    }
    if (!a->credit)
    {
        if (!t->blocked)
        {
            t->blocked = 1;
            t->blocked_since = now;
        }
        t->credit = 0;
        t->due = now + 100;
        return 0;
    }
    t->credit = 1;
    t->blocked = 0;
    t->due = now;
    return 0;
}

int edu_rx_accept(struct edu_rx *r, const struct edu_fragment *f,
                  struct edu_fragment *ack)
{
    if (!fragment_valid(f) || f->type == 2)
    {
        return -1;
    }
    *ack = (struct edu_fragment){
        2,        f->destination_port, f->source_port, f->index,
        f->count, (uint8_t)!r->ready,  f->id,          {0},
        0};
    if (f->type == 3)
    {
        return 0;
    }
    if (f->id <= r->completed)
    {
        if (f->id == r->completed && f->source_port == r->source &&
            f->destination_port == r->destination &&
            f->count == r->last_count && f->index + 1 == f->count &&
            f->length == r->last_length && !memcmp(f->data, r->last, f->length))
        {
            ack->credit = (uint8_t)!r->ready;
            return 0;
        }
        return -1;
    }
    /* Never acknowledge an unaccepted DATA fragment merely to advertise credit.
     */
    if (r->ready)
    {
        ack->credit = 0;
        return -1;
    }
    if (!r->active)
    {
        if (f->index)
        {
            return -1;
        }
        r->active = 1;
        r->id = f->id;
        r->source = f->source_port;
        r->destination = f->destination_port;
        r->count = f->count;
        r->index = 0;
        r->length = 0;
    }
    if (f->id != r->id || f->source_port != r->source ||
        f->destination_port != r->destination || f->count != r->count)
    {
        return -1;
    }
    if (f->index < r->index)
    {
        size_t pos = (size_t)f->index * 24;
        if (pos + f->length > r->length ||
            memcmp(r->bytes + pos, f->data, f->length))
        {
            return -1;
        }
        return 0;
    }
    if (f->index != r->index)
    {
        return -1;
    }
    memcpy(r->bytes + r->length, f->data, f->length);
    r->length += f->length;
    r->index++;
    if (r->index == r->count)
    {
        r->ready = 1;
        r->active = 0;
        r->completed = f->id;
        r->last_count = f->count;
        r->last_length = f->length;
        memcpy(r->last, f->data, f->length);
        r->publications++;
        ack->credit = 0;
        return 1;
    }
    return 0;
}

void edu_rx_release(struct edu_rx *r)
{
    r->ready = 0;
}

static int utf8(const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n;)
    {
        uint32_t cp;
        unsigned extra;
        uint8_t c = p[i++];
        if (c < 128)
        {
            continue;
        }
        if (c >= 0xc2 && c <= 0xdf)
        {
            cp = c & 31;
            extra = 1;
        }
        else if (c >= 0xe0 && c <= 0xef)
        {
            cp = c & 15;
            extra = 2;
        }
        else if (c >= 0xf0 && c <= 0xf4)
        {
            cp = c & 7;
            extra = 3;
        }
        else
        {
            return 0;
        }
        if (extra > n - i)
        {
            return 0;
        }
        unsigned k = extra;
        while (k--)
        {
            c = p[i++];
            if ((c & 0xc0) != 0x80)
            {
                return 0;
            }
            cp = (cp << 6) | (c & 63);
        }
        if ((extra == 1 && cp < 128) || (extra == 2 && cp < 2048) ||
            (extra == 3 && cp < 65536) || cp > 0x10ffff ||
            (cp >= 0xd800 && cp <= 0xdfff))
        {
            return 0;
        }
    }
    return 1;
}

int edu_record_decode(const void *v, size_t n, struct edu_record *out)
{
    const uint8_t *p = v;
    if (!n || n > 80 || p[0] != 1)
    {
        return -1;
    }
    struct edu_record r = {0};
    unsigned seen = 0, prior = 0;
    for (size_t i = 1; i < n;)
    {
        if (n - i < 2)
        {
            return -1;
        }
        unsigned tag = p[i++], len = p[i++];
        if (tag <= prior || tag > 5 || len > n - i)
        {
            return -1;
        }
        prior = tag;
        seen |= 1u << tag;
        if (tag == 1)
        {
            if (len != 1 || p[i] < 1 || p[i] > 4)
            {
                return -1;
            }
            r.verb = p[i];
        }
        if (tag == 2)
        {
            if (len != 2)
            {
                return -1;
            }
            r.request = be16(p + i);
        }
        if (tag == 3)
        {
            if (len != 2)
            {
                return -1;
            }
            unsigned u = be16(p + i);
            r.value = (int16_t)(u < 32768 ? (int)u : (int)u - 65536);
            r.has_value = 1;
        }
        if (tag == 4)
        {
            if (len != 1 || p[i] != 1)
            {
                return -1;
            }
            r.has_unit = 1;
        }
        if (tag == 5)
        {
            if (len > 16 || !utf8(p + i, len))
            {
                return -1;
            }
            memcpy(r.label, p + i, len);
            r.label_length = (uint8_t)len;
        }
        i += len;
    }
    if ((seen & 6) != 6 ||
        ((r.verb == 2 || r.verb == 3) && (!r.has_value || !r.has_unit)) ||
        (r.verb == 1 && (r.has_value || r.has_unit)) ||
        r.has_value != r.has_unit)
    {
        return -1;
    }
    *out = r;
    return 0;
}

int edu_record_encode(const struct edu_record *r, void *v, size_t *n)
{
    uint8_t p[33];
    size_t i = 0;
    p[i++] = 1;
    p[i++] = 1;
    p[i++] = 1;
    p[i++] = r->verb;
    p[i++] = 2;
    p[i++] = 2;
    put16(p + i, r->request);
    i += 2;
    if (r->has_value)
    {
        p[i++] = 3;
        p[i++] = 2;
        put16(p + i, (uint16_t)r->value);
        i += 2;
    }
    if (r->has_unit)
    {
        p[i++] = 4;
        p[i++] = 1;
        p[i++] = 1;
    }
    if (r->label_length)
    {
        if (r->label_length > 16)
        {
            return -1;
        }
        p[i++] = 5;
        p[i++] = r->label_length;
        memcpy(p + i, r->label, r->label_length);
        i += r->label_length;
    }
    struct edu_record check;
    if (edu_record_decode(p, i, &check))
    {
        return -1;
    }
    memcpy(v, p, i);
    *n = i;
    return 0;
}

int edu_application(const struct edu_record *r, int16_t *set,
                    struct edu_record *reply)
{
    if (r->verb != 1 && r->verb != 2)
    {
        return -1;
    }
    *reply = (struct edu_record){3, r->request, 1800, 1, 1, {0}, 0};
    if (r->verb == 2)
    {
        if (!r->has_value || !r->has_unit)
        {
            return -1;
        }
        if (r->value < -4000 || r->value > 12500)
        {
            reply->verb = 4;
        }
        else
        {
            *set = r->value;
        }
        reply->value = *set;
    }
    return 0;
}

int edu_session_receive(struct edu_session *s, uint8_t sender, const void *v,
                        size_t n, const uint8_t **data, size_t *len)
{
    const uint8_t *p = v;
    if (n < 7 || n > 87 || !(sender == 17 || sender == 51))
    {
        return -1;
    }
    uint16_t id = be16(p + 1), epoch = be16(p + 3), cp = be16(p + 5);
    if (!id || !epoch)
    {
        return -1;
    }
    if (p[0] == 1 || p[0] == 2)
    {
        if (cp || n != 10 || p[7] != 1 || p[8] != 1 || p[9] != 17)
        {
            return -1;
        }
        if (s->open && (s->session != id || s->epoch != epoch))
        {
            return -1;
        }
        if (!s->open)
        {
            memset(s, 0, sizeof *s);
            s->session = id;
            s->epoch = epoch;
            s->holder = 17;
            s->open = 1;
        }
        return 0;
    }
    if (!s->open || s->session != id || s->epoch != epoch)
    {
        return -1;
    }
    if (p[0] == 3)
    {
        if (!cp || n == 7)
        {
            return -1;
        }
        if (cp == s->checkpoint)
        {
            return n - 7 == s->last_length && !memcmp(p + 7, s->last, n - 7)
                       ? 2
                       : -1;
        }
        if (s->holder != sender || s->token_pending || s->checkpoint == 65535 ||
            cp != s->checkpoint + 1)
        {
            return -1;
        }
        s->checkpoint = cp;
        s->last_length = n - 7;
        memcpy(s->last, p + 7, n - 7);
        s->effects++;
        *data = p + 7;
        *len = n - 7;
        return 1;
    }
    if (p[0] == 4)
    {
        return n == 7 && cp == s->checkpoint ? 0 : -1;
    }
    if (cp)
    {
        return -1;
    }
    if (p[0] == 5)
    {
        if (n != 8 || !(p[7] == 17 || p[7] == 51) || p[7] == sender ||
            (s->holder != sender && s->holder != p[7]))
        {
            return -1;
        }
        s->holder = p[7];
        return 0;
    }
    if (p[0] == 6)
    {
        if (n != 7)
        {
            return -1;
        }
        s->open = 0;
        return 0;
    }
    return p[0] == 7 && n == 7 ? 0 : -1;
}

int edu_session_token_begin(struct edu_session *s, uint8_t sender)
{
    if (!s->open || s->holder != sender || s->token_pending)
    {
        return -1;
    }
    s->token_pending = 1;
    return 0;
}

void edu_session_token_complete(struct edu_session *s, uint8_t holder, int ok)
{
    if (s->token_pending && ok && (holder == 17 || holder == 51))
    {
        s->holder = holder;
        s->token_pending = 0;
    }
}

int edu_phy_begin(struct edu_physical *p, uint64_t now)
{
    if (!edu_phy_ready(p, now))
    {
        return -1;
    }
    p->driving = 1;
    p->phase = 1;
    p->due = now + 200;
    return 0;
}

int edu_phy_started(struct edu_physical *p, uint64_t now)
{
    if (p->phase != 1 || now < p->due)
    {
        return -1;
    }
    p->phase = 2;
    return 0;
}

int edu_phy_complete(struct edu_physical *p, uint64_t now, int tc)
{
    if (p->phase != 2 || !tc)
    {
        return -1;
    }
    p->driving = 0;
    p->phase = 3;
    p->due = now + 200;
    return 0;
}

int edu_phy_ready(struct edu_physical *p, uint64_t now)
{
    if (p->phase == 3 && now >= p->due)
    {
        p->phase = 0;
    }
    return p->phase == 0;
}
