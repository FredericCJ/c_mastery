#include "checkpoint_depth.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "P3 wire requires octets");
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif

uint32_t cp_crc32(const unsigned char *bytes, size_t n)
{
    uint32_t crc = UINT32_MAX;
    for (size_t i = 0; i < n; ++i)
    {
        crc ^= bytes[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1u) ? UINT32_C(0xedb88320) : 0u);
    }
    return crc ^ UINT32_MAX;
}

static void copy_message(struct cp_message *to, const struct cp_message *from)
{
    to->sequence = from->sequence;
    to->length = from->length;
    for (size_t i = 0; i < from->length; ++i)
        to->payload[i] = from->payload[i];
}

enum cp_status cp_encode(const struct cp_message *m, unsigned char *out,
                         size_t capacity, size_t *written)
{
    if (!m || !written || m->length > CP_PAYLOAD || !out)
        return CP_INVALID;
    size_t total = 10 + m->length;
    if (capacity < total)
        return CP_FULL;
    out[0] = 0xa5;
    out[1] = 1;
    out[2] = (unsigned char)m->sequence;
    out[3] = (unsigned char)(m->sequence >> 8);
    if (CP_MUTANT == 2)
    {
        out[2] = (unsigned char)(m->sequence >> 8);
        out[3] = (unsigned char)m->sequence;
    }
    out[4] = (unsigned char)m->length;
    out[5] = 0;
    for (size_t i = 0; i < m->length; ++i)
        out[6 + i] = m->payload[i];
    uint32_t crc = cp_crc32(out + 1, 5 + m->length);
    if (CP_MUTANT == 1)
        crc = cp_crc32(out + 2, 4 + m->length);
    for (unsigned i = 0; i < 4; ++i)
        out[6 + m->length + i] = (unsigned char)(crc >> (8 * i));
    *written = total;
    return CP_OK;
}

void cp_parser_init(struct cp_parser *p)
{
    p->used = 0;
    p->ready = 0;
}

static void remove_prefix(struct cp_parser *p, size_t n)
{
    for (size_t i = n; i < p->used; ++i)
        p->candidate[i - n] = p->candidate[i];
    p->used -= n;
}

static enum cp_status inspect(struct cp_parser *p)
{
    while (p->used && p->candidate[0] != 0xa5)
        remove_prefix(p, 1);
    if (p->used < 2)
        return CP_NEED_MORE;
    if (p->candidate[1] != 1)
    {
        remove_prefix(p, 1);
        return CP_REJECT;
    }
    if (p->used < 6)
        return CP_NEED_MORE;
    size_t n = (size_t)p->candidate[4] + ((size_t)p->candidate[5] << 8);
    if (n > CP_PAYLOAD)
    {
        remove_prefix(p, 1);
        return CP_REJECT;
    }
    if (p->used < n + 10)
        return CP_NEED_MORE;
    uint32_t received = 0;
    for (unsigned i = 0; i < 4; ++i)
        received |= (uint32_t)p->candidate[6 + n + i] << (8 * i);
    if (received != cp_crc32(p->candidate + 1, n + 5))
    {
        remove_prefix(p, CP_MUTANT == 3 ? n + 10 : 1);
        return CP_REJECT;
    }
    p->message.sequence = (uint16_t)((unsigned)p->candidate[2] |
                                     ((unsigned)p->candidate[3] << 8));
    p->message.length = n;
    for (size_t i = 0; i < n; ++i)
        p->message.payload[i] = p->candidate[6 + i];
    remove_prefix(p, n + 10);
    p->ready = 1;
    return CP_FRAME;
}

enum cp_status cp_feed(struct cp_parser *p, const unsigned char *bytes,
                       size_t n, size_t *used)
{
    if (!p || !used || (!bytes && n))
        return CP_INVALID;
    *used = 0;
    if (p->ready)
        return CP_FRAME;
    for (;;)
    {
        enum cp_status result = inspect(p);
        if (result != CP_NEED_MORE)
            return result;
        if (*used == n)
            return CP_NEED_MORE;
        /* inspect consumes or rejects every complete candidate before this append. */
        if (p->used == CP_WIRE)
            return CP_INVALID;
        p->candidate[p->used++] = bytes[(*used)++];
    }
}

enum cp_status cp_peek(const struct cp_parser *p, struct cp_message *out)
{
    if (!p || !out)
        return CP_INVALID;
    if (!p->ready)
        return CP_NEED_MORE;
    copy_message(out, &p->message);
    return CP_FRAME;
}

void cp_queue_init(struct cp_queue *q)
{
    q->head = 0;
    q->count = 0;
}

enum cp_status cp_deliver(struct cp_parser *p, struct cp_queue *q)
{
    if (!p || !q)
        return CP_INVALID;
    if (!p->ready)
        return CP_NEED_MORE;
    if (q->count == CP_QUEUE)
    {
        if (CP_MUTANT == 4)
            p->ready = 0;
        return CP_FULL;
    }
    copy_message(&q->items[(q->head + q->count) % CP_QUEUE], &p->message);
    ++q->count;
    p->ready = 0;
    return CP_OK;
}

enum cp_status cp_take(struct cp_queue *q, struct cp_message *out)
{
    if (!q || !out)
        return CP_INVALID;
    if (!q->count)
        return CP_NEED_MORE;
    copy_message(out, &q->items[q->head]);
    q->head = (q->head + 1) % CP_QUEUE;
    --q->count;
    return CP_OK;
}
