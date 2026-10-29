#include "logical.h"
#include <limits.h>
#include <string.h>
#ifndef SL_MUTANT
#define SL_MUTANT 0
#endif
#define MAGIC UINT32_C(0x534c5231)

static uint32_t crc(const uint8_t *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; i++)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; j++)
        {
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
        }
    }
    return ~c;
}

static uint32_t r32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static uint64_t r64(const uint8_t *p)
{
    return (uint64_t)r32(p) | (uint64_t)r32(p + 4) << 32;
}

static int64_t signed64(uint64_t x)
{
    return x <= INT64_MAX ? (int64_t)x : -1 - (int64_t)(UINT64_MAX - x);
}

static void w32(uint8_t *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

static void w64(uint8_t *p, uint64_t v)
{
    w32(p, (uint32_t)v);
    w32(p + 4, (uint32_t)(v >> 32));
}

static int add(int64_t a, int64_t b, int64_t *out)
{
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b))
    {
        return 1;
    }
    *out = a + b;
    return 0;
}

static int valid_io(store_io i)
{
    return i.page_count >= 3 && i.read && i.write && i.flush;
}

/* Snapshot: id,total,last_delta. Command: id,base_id,delta; NO after-total. */
static void encode(uint8_t *p, unsigned kind, uint64_t id, uint64_t value,
                   int64_t delta)
{
    memset(p, 0, 4096);
    w32(p, MAGIC);
    w32(p + 4, kind);
    w64(p + 8, id);
    w64(p + 16, value);
    w64(p + 24, (uint64_t)delta);
    w32(p + 4092, crc(p, 4092));
}

static int valid(const uint8_t *p, unsigned kind)
{
    if (r32(p) != MAGIC || r32(p + 4) != kind || r32(p + 4092) != crc(p, 4092))
    {
        return 0;
    }
    for (unsigned i = 32; i < 4092; i++)
    {
        if (p[i])
        {
            return 0;
        }
    }
    uint64_t id = r64(p + 8);
    if (kind == 2)
    {
        return id && r64(p + 16) == id - 1;
    }
    return id || (!r64(p + 16) && !r64(p + 24));
}

static int writep(store_io io, unsigned p, const void *b)
{
    return io.write(io.ctx, p, b) ? SL_IO : SL_OK;
}

static int flush(store_io io)
{
    return io.flush(io.ctx) ? SL_IO : SL_OK;
}

static int clear_log(store_io io)
{
    uint8_t zero[4096] = {0};
    return writep(io, 2, zero) || flush(io) ? SL_IO : SL_OK;
}

int sl_format(store_io io)
{
    if (!valid_io(io))
    {
        return SL_INVALID;
    }
    uint8_t b[4096];
    encode(b, 1, 0, 0, 0);
    if (clear_log(io) || writep(io, 0, b) || flush(io) || writep(io, 1, b) ||
        flush(io))
    {
        return SL_IO;
    }
    return SL_OK;
}

static int replay(struct sl_counter *s, const uint8_t *log)
{
    uint64_t id = r64(log + 8);
    int64_t delta = signed64(r64(log + 24));
    if (id < s->id)
    {
        return SL_OK;
    }
    if (id == s->id && SL_MUTANT != 2)
    {
        if (delta != s->delta)
        {
            return SL_CORRUPT;
        }
        return clear_log(s->io);
    }
    if (SL_MUTANT != 2 &&
        (s->id == UINT64_MAX || id != s->id + 1 || r64(log + 16) != s->id))
    {
        return SL_CORRUPT;
    }
    int64_t total;
    if (add(s->total, delta, &total))
    {
        return SL_CORRUPT;
    }
    uint8_t bytes[4096];
    encode(bytes, 1, id, (uint64_t)total, delta);
    if (writep(s->io, (unsigned)(id & 1u), bytes) || flush(s->io) ||
        clear_log(s->io))
    {
        return SL_IO;
    }
    s->id = id;
    s->total = total;
    s->delta = delta;
    return SL_OK;
}

int sl_open(struct sl_counter *s, store_io io)
{
    if (!s || !valid_io(io))
    {
        return SL_INVALID;
    }
    memset(s, 0, sizeof *s);
    s->io = io;
    s->poisoned = 1;
    uint8_t a[4096], b[4096], log[4096];
    if (io.read(io.ctx, 0, a) || io.read(io.ctx, 1, b) ||
        io.read(io.ctx, 2, log))
    {
        return SL_IO;
    }
    int va = valid(a, 1), vb = valid(b, 1);
    if (!va && !vb)
    {
        return SL_CORRUPT;
    }
    if (va && vb && r64(a + 8) == r64(b + 8) && memcmp(a, b, 4096))
    {
        return SL_CORRUPT;
    }
    const uint8_t *p = !vb || (va && r64(a + 8) >= r64(b + 8)) ? a : b;
    s->id = r64(p + 8);
    s->total = signed64(r64(p + 16));
    s->delta = signed64(r64(p + 24));
    if (valid(log, 2))
    {
        int r = replay(s, log);
        if (r)
        {
            return r;
        }
    }
    s->poisoned = 0;
    return SL_OK;
}

int sl_get(const struct sl_counter *s, struct sl_receipt *out)
{
    if (!s || !out)
    {
        return SL_INVALID;
    }
    if (s->poisoned)
    {
        return SL_STATE;
    }
    *out = (struct sl_receipt){s->id, s->total};
    return SL_OK;
}

int sl_checkpoint(struct sl_counter *s)
{
    if (!s || s->poisoned)
    {
        return SL_STATE;
    }
    if (!s->pending)
    {
        return SL_OK;
    }
    uint8_t bytes[4096];
    encode(bytes, 1, s->id, (uint64_t)s->total, s->delta);
    if (writep(s->io, (unsigned)(s->id & 1u), bytes) || flush(s->io) ||
        clear_log(s->io))
    {
        s->poisoned = 1;
        return SL_IO;
    }
    s->pending = 0;
    return SL_OK;
}

int sl_apply(struct sl_counter *s, uint64_t id, int64_t delta,
             struct sl_receipt *out)
{
    if (!s || !out || !id)
    {
        return SL_INVALID;
    }
    if (s->poisoned)
    {
        return SL_STATE;
    }
    if (id == s->id)
    {
        if (SL_MUTANT != 4 && delta != s->delta)
        {
            return SL_CONFLICT;
        }
        *out = (struct sl_receipt){id, s->total};
        return SL_DUPLICATE;
    }
    if (id < s->id)
    {
        return SL_STALE;
    }
    if (s->id == UINT64_MAX)
    {
        return SL_FULL;
    }
    if (id != s->id + 1)
    {
        return SL_GAP;
    }
    int64_t total;
    if (add(s->total, delta, &total))
    {
        if (SL_MUTANT != 5)
        {
            return SL_FULL;
        }
        total = signed64((uint64_t)s->total + (uint64_t)delta);
    }
    int r = sl_checkpoint(s);
    if (r)
    {
        return r;
    }
    if (clear_log(s->io))
    {
        s->poisoned = 1;
        return SL_IO;
    }
    uint8_t log[4096];
    encode(log, 2, id, s->id, delta);
    if (SL_MUTANT == 3)
    {
        *out = (struct sl_receipt){id, total};
    }
    r = writep(s->io, 2, log);
    if (!r && SL_MUTANT != 1)
    {
        r = flush(s->io);
    }
    if (r)
    {
        s->poisoned = 1;
        return SL_UNCERTAIN;
    }
    s->id = id;
    s->total = total;
    s->delta = delta;
    s->pending = 1;
    *out = (struct sl_receipt){id, total};
    return SL_OK;
}
