#include "stream.h"

int rf_datagram_accept(size_t reported, size_t capacity, unsigned flags)
{
    if (flags & ~(unsigned)(RF_PAYLOAD_TRUNCATED | RF_CONTROL_TRUNCATED))
        return -1;
#ifdef RF_MUTANT_TRUNCATION
    (void)flags;
    return reported <= capacity;
#else
    return !flags && reported <= capacity;
#endif
}

int rf_parser_init(struct rf_parser *p, unsigned width)
{
    if (!p || (width != 1 && width != 2))
        return -1;
    *p = (struct rf_parser){.width = width};
    return 0;
}

int rf_push(struct rf_parser *p, unsigned char byte, struct rf_frame *out)
{
    if (!p || !out || p->failed || (p->width != 1 && p->width != 2) ||
        p->header_used > p->width || p->used > 8)
        return -1;
    if (p->header_used < p->width)
    {
        p->length = p->length * 256u + byte;
        ++p->header_used;
        if (p->header_used < p->width)
            return 0;
#ifndef RF_MUTANT_LENGTH
        if (p->length > 8)
        {
            p->failed = 1;
            return -1;
        }
#else
        p->length %= 9;
#endif
    }
    else
    {
        if (p->used >= p->length || p->length > 8)
        {
            p->failed = 1;
            return -1;
        }
        p->payload[p->used++] = byte;
    }
    if (p->used != p->length)
        return 0;
    struct rf_frame next = {.length = p->length};
    for (unsigned i = 0; i < p->length; ++i)
        next.bytes[i] = p->payload[i];
    *out = next;
    p->header_used = p->length = p->used = 0;
    return 1;
}

int rf_feed(struct rf_parser *p, const unsigned char *bytes, size_t length,
            size_t *consumed, struct rf_frame *out)
{
    if (!p || !consumed || !out || (!bytes && length))
        return -1;
    *consumed = 0;
    for (size_t i = 0; i < length; ++i)
    {
        int r = rf_push(p, bytes[i], out);
        *consumed = i + 1;
        if (r)
            return r;
    }
    return 0;
}

int rf_eof(struct rf_parser *p)
{
    if (!p || (p->width != 1 && p->width != 2))
        return -1;
#ifdef RF_MUTANT_EOF
    return p->failed ? -1 : 0;
#else
    if (p->failed || p->header_used || p->used)
    {
        p->failed = 1;
        return -1;
    }
    return 0;
#endif
}

int rf_message_init(struct rf_message *m, unsigned id, unsigned width,
                    const struct rf_frame *frame)
{
    if (!m || !frame || !id || id > 65535 || frame->length > 8 ||
        (width != 1 && width != 2))
        return -1;
    struct rf_message next = {.id = id,
                              .payload_length = frame->length,
                              .wire_length = width + frame->length,
                              .owner = RF_CALLER};
    next.wire[width - 1] = (unsigned char)frame->length;
    for (unsigned i = 0; i < frame->length; ++i)
        next.wire[width + i] = frame->bytes[i];
    *m = next;
    return 0;
}

int rf_outbox_init(struct rf_outbox *q, size_t limit)
{
    if (!q || limit > 256)
        return -1;
    *q = (struct rf_outbox){.limit = limit};
    return 0;
}

int rf_outbox_invariant(const struct rf_outbox *q)
{
    if (!q || q->limit > 256 || q->bytes > q->limit)
        return 0;
    size_t bytes = 0;
    for (unsigned c = 0; c < 4; ++c)
    {
        const struct rf_connection *p = &q->connections[c];
        if (p->active > 1 || p->head > 7 || p->count > 8 ||
            (!p->active && p->count))
            return 0;
        for (unsigned i = 0; i < p->count; ++i)
        {
            struct rf_message *m = p->items[(p->head + i) % 8];
            if (!m || m->owner != RF_OUTBOX || !m->id || m->id > 65535 ||
                m->payload_length > 8 ||
                m->wire_length < m->payload_length + 1 ||
                m->wire_length > m->payload_length + 2 ||
                m->offset >= m->wire_length)
                return 0;
            bytes += m->payload_length;
            for (unsigned earlier = 0; earlier <= c; ++earlier)
            {
                const struct rf_connection *a = &q->connections[earlier];
                unsigned limit = earlier == c ? i : a->count;
                for (unsigned j = 0; j < limit; ++j)
                {
                    struct rf_message *v = a->items[(a->head + j) % 8];
                    if (v == m || v->id == m->id)
                        return 0;
                }
            }
        }
    }
    return bytes == q->bytes;
}

int rf_open(struct rf_outbox *q, unsigned *slot)
{
    if (!slot || !rf_outbox_invariant(q))
        return -1;
    for (unsigned i = 0; i < 4; ++i)
        if (!q->connections[i].active)
        {
            q->connections[i].active = 1;
            *slot = i;
            return 1;
        }
    return 0;
}

int rf_enqueue(struct rf_outbox *q, unsigned slot, struct rf_message *m)
{
    if (!rf_outbox_invariant(q) || slot > 3 || !q->connections[slot].active ||
        !m || m->owner != RF_CALLER || !m->id || m->id > 65535 || m->offset ||
        m->payload_length > 8 || m->wire_length < m->payload_length + 1 ||
        m->wire_length > m->payload_length + 2)
        return -1;
    for (unsigned c = 0; c < 4; ++c)
        for (unsigned j = 0; j < q->connections[c].count; ++j)
        {
            struct rf_message *v =
                q->connections[c].items[(q->connections[c].head + j) % 8];
            if (v == m || v->id == m->id)
                return -1;
        }
    struct rf_connection *p = &q->connections[slot];
    if (p->count == 8)
        return 0;
#ifndef RF_MUTANT_GLOBAL
    if (m->payload_length > q->limit - q->bytes)
        return 0;
#endif
    p->items[(p->head + p->count) % 8] = m;
    ++p->count;
    q->bytes += m->payload_length;
    m->owner = RF_OUTBOX;
    return 1;
}

int rf_peek(struct rf_outbox *q, unsigned slot, const unsigned char **bytes,
            size_t *length)
{
    if (!bytes || !length || !rf_outbox_invariant(q) || slot > 3 ||
        !q->connections[slot].active)
        return -1;
    struct rf_connection *p = &q->connections[slot];
    if (!p->count)
        return 0;
    struct rf_message *m = p->items[p->head];
    *bytes = m->wire + m->offset;
    *length = m->wire_length - m->offset;
    return 1;
}

int rf_advance(struct rf_outbox *q, unsigned slot, size_t n)
{
    if (!rf_outbox_invariant(q) || slot > 3 || !q->connections[slot].active ||
        !n)
        return -1;
    struct rf_connection *p = &q->connections[slot];
    if (!p->count)
        return -1;
    struct rf_message *m = p->items[p->head];
    if (n > m->wire_length - m->offset)
        return -1;
    m->offset += (unsigned)n;
    if (m->offset == m->wire_length)
    {
        m->owner = RF_DELIVERED;
        q->bytes -= m->payload_length;
        p->items[p->head] = 0;
        p->head = (p->head + 1) % 8;
        --p->count;
    }
    return 0;
}

int rf_send_once(struct rf_outbox *q, unsigned slot,
                 int (*writer)(void *, const unsigned char *, size_t),
                 void *context)
{
    if (!writer)
        return -1;
    const unsigned char *bytes;
    size_t length;
    int ready = rf_peek(q, slot, &bytes, &length);
    if (ready <= 0)
        return ready;
    int n = writer(context, bytes, length);
    if (n < 0)
        return -1;
    if (!n)
        return 0;
    if ((size_t)n > length)
        return -1;
#ifdef RF_MUTANT_ADVANCE_ALL
    return rf_advance(q, slot, length) == 0 ? n : -1;
#else
    return rf_advance(q, slot, (size_t)n) == 0 ? n : -1;
#endif
}

int rf_disconnect(struct rf_outbox *q, unsigned slot)
{
    if (!rf_outbox_invariant(q) || slot > 3)
        return -1;
    struct rf_connection *p = &q->connections[slot];
    while (p->count)
    {
        struct rf_message *m = p->items[p->head];
        q->bytes -= m->payload_length;
        m->owner = RF_CANCELLED;
        p->items[p->head] = 0;
        p->head = (p->head + 1) % 8;
        --p->count;
    }
    p->active = 0;
    return 0;
}

int rf_dedup_init(struct rf_dedup *d, unsigned session)
{
    if (!d || !session || session > 65535)
        return -1;
    *d = (struct rf_dedup){.session = session};
    return 0;
}

int rf_effect(struct rf_dedup *d, unsigned session, unsigned id, unsigned delta,
              uint32_t *result)
{
    if (!d || !result || !session || d->session != session || !id ||
        id > 65535 || delta > 255 || d->count > 4)
        return -1;
    for (unsigned i = 0; i < d->count; ++i)
        if (d->cache[i].id == id)
        {
            if (d->cache[i].delta != delta)
                return -1;
#ifndef RF_MUTANT_REPLAY
#ifdef RF_MUTANT_CURRENT_RESULT
            *result = d->total;
#else
            *result = d->cache[i].result;
#endif
            return 0;
#endif
        }
    if (d->count == 4 || UINT32_MAX - d->total < delta)
        return -1;
    d->total += delta;
    d->cache[d->count++] = (struct rf_cached){id, delta, d->total};
    *result = d->total;
    return 1;
}

int rf_sequence_init(struct rf_sequence *s, unsigned session, unsigned first)
{
    if (!s || !session || session > 65535 || first > 65535)
        return -1;
    *s = (struct rf_sequence){.session = session, .next = first};
    return 0;
}

static void drain(struct rf_sequence *s, unsigned delivered[3], unsigned *count)
{
    while (s->pending & 1u)
    {
        delivered[(*count)++] = s->next;
        for (unsigned i = 2; i > 0; --i)
            s->recent[i] = s->recent[i - 1];
        s->recent[0] = s->next;
        if (s->recent_count < 3)
            ++s->recent_count;
        ++s->next;
        s->pending >>= 1;
    }
}

int rf_arrive(struct rf_sequence *s, unsigned session, unsigned seq,
              unsigned delivered[3], unsigned *count)
{
    if (!s || !delivered || !count || !s->session || s->session > 65535 ||
        s->next > 65536 || s->pending > 7 || s->recent_count > 3 || seq > 65535)
        return RF_SEQ_INVALID;
    *count = 0;
#ifndef RF_MUTANT_SESSION
    if (session != s->session)
        return RF_SEQ_SESSION;
#else
    (void)session;
#endif
    for (unsigned i = 0; i < s->recent_count; ++i)
        if (s->recent[i] == seq)
            return RF_SEQ_DUPLICATE;
    if (s->next == 65536)
        return RF_SEQ_RETIRED;
    if (seq < s->next)
        return RF_SEQ_STALE;
    if (seq - s->next >= 3)
        return RF_SEQ_WINDOW;
    unsigned bit = 1u << (seq - s->next);
    if (s->pending & bit)
        return RF_SEQ_DUPLICATE;
    s->pending |= bit;
    drain(s, delivered, count);
    return RF_SEQ_ACCEPT;
}

int rf_expire_gap(struct rf_sequence *s, unsigned *missing,
                  unsigned delivered[3], unsigned *count)
{
    if (!s || !missing || !delivered || !count || !s->session ||
        s->session > 65535 || s->next > 65535 || s->pending > 7 ||
        s->recent_count > 3)
        return -1;
    *count = 0;
    if (!s->pending || (s->pending & 1u))
        return 0;
    *missing = s->next++;
    s->pending >>= 1;
    drain(s, delivered, count);
    return 1;
}
