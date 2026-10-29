#include "algorithms.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8,
               "Wire octets require eight-bit bytes in this adapter");
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
enum ad_status ad_record_encode(const struct ad_wire_record *r,
                                unsigned char *p, size_t n)
{
    if (!r || !p || n != 8 || (r->flags & ~3u))
    {
        return AD_INVALID;
    }
    uint32_t value = (uint32_t)r->value;
    unsigned char bytes[8] = {1,
                              r->flags,
                              (unsigned char)(r->count >> 8),
                              (unsigned char)r->count,
                              (unsigned char)(value >> 24),
                              (unsigned char)(value >> 16),
                              (unsigned char)(value >> 8),
                              (unsigned char)value};
    for (unsigned i = 0; i < 8; ++i)
    {
        p[i] = bytes[i];
    }
    return AD_OK;
}

enum ad_status ad_record_decode(const unsigned char *p, size_t n,
                                struct ad_wire_record *out)
{
    if (!p || !out || n != 8 || p[0] != 1 || (p[1] & ~3u))
    {
        return AD_INVALID;
    }
    uint32_t raw = ((uint32_t)p[4] << 24) | ((uint32_t)p[5] << 16) |
                   ((uint32_t)p[6] << 8) | p[7];
    int32_t value =
        raw <= INT32_MAX ? (int32_t)raw : -(int32_t)(UINT32_MAX - raw) - 1;
    *out = (struct ad_wire_record){
        p[1], (uint16_t)((unsigned)p[2] * 256u + p[3]), value};
    return AD_OK;
}

enum ad_status ad_record_extensions(const unsigned char *p, size_t n,
                                    struct ad_wire_record *out,
                                    unsigned char *annotation,
                                    unsigned *present)
{
    if (!p || n < 8 || n > 64 || !out || !annotation || !present)
    {
        return AD_INVALID;
    }
    struct ad_wire_record record;
    enum ad_status rc = ad_record_decode(p, 8, &record);
    if (rc != AD_OK)
    {
        return rc;
    }
    unsigned seen = 0;
    unsigned char note = 0;
    size_t at = 8;
    while (at < n)
    {
        if (n - at < 2)
        {
            return AD_INCOMPLETE;
        }
        unsigned type = p[at++], length = p[at++];
        if (length > n - at)
        {
            return AD_INCOMPLETE;
        }
        if (type == 1)
        {
            if (seen || length != 1)
            {
                return AD_INVALID;
            }
            seen = 1;
            note = p[at];
        }
        else if (type & 128u)
        {
            return AD_INVALID;
        }
        at += length;
    }
    *out = record;
    *annotation = note;
    *present = seen;
    return AD_OK;
}

unsigned char ad_crc8(const unsigned char *p, size_t n, unsigned reflected,
                      unsigned char initial, unsigned char final_xor)
{
    unsigned value = initial;
    for (size_t i = 0; i < n; ++i)
    {
        value ^= p[i];
        for (unsigned b = 0; b < 8; ++b)
        {
            if (reflected)
            {
                value = (value >> 1) ^ ((value & 1u) ? 0xe0u : 0u);
            }
            else
            {
                value = ((value << 1) ^ ((value & 128u) ? 7u : 0u)) & 255u;
            }
        }
    }
    return (unsigned char)(value ^ final_xor);
}

unsigned char ad_crc8_table(const unsigned char *p, size_t n)
{
    unsigned char table[256];
    for (unsigned i = 0; i < 256; ++i)
    {
        unsigned char byte = (unsigned char)i;
        table[i] = ad_crc8(&byte, 1, 0, 0, 0);
    }
    unsigned char result = 0;
    for (size_t i = 0; i < n; ++i)
    {
        result = table[result ^ p[i]];
    }
    return result;
}

enum ad_status ad_frame_init(struct ad_framer *f, unsigned mode)
{
    if (!f || mode > 1)
    {
        return AD_INVALID;
    }
    *f = (struct ad_framer){.mode = mode};
    return AD_OK;
}

enum ad_status ad_frame_feed(struct ad_framer *f, const unsigned char *p,
                             size_t n, size_t *consumed)
{
    if (!f || !consumed || (n && !p) || n > 64)
    {
        return AD_INVALID;
    }
    *consumed = 0;
    if (f->state == 2)
    {
        if (AD_MUTANT == 7 && n)
        {
            *consumed = 1;
        }
        return AD_READY;
    }
    if (f->state >= 3)
    {
        return AD_FAILED;
    }
    while (*consumed < n)
    {
        unsigned char byte = p[(*consumed)++];
        if (!f->mode)
        {
            if (f->state == 0)
            {
                if (byte > 8)
                {
                    f->state = 3;
                    return AD_INVALID;
                }
                f->expected = byte;
                f->used = 0;
                f->state = byte ? 1 : 2;
                if (!byte)
                {
                    return AD_READY;
                }
            }
            else
            {
                f->bytes[f->used++] = byte;
                if (f->used == f->expected)
                {
                    f->state = 2;
                    return AD_READY;
                }
            }
        }
        else
        {
            if (!f->escaped && byte == 0x7e)
            {
                f->state = 2;
                return AD_READY;
            }
            if (!f->escaped && byte == 0x7d)
            {
                f->escaped = 1;
                continue;
            }
            if (f->escaped)
            {
                byte ^= 0x20;
                f->escaped = 0;
            }
            if (f->used == 8)
            {
                f->state = 3;
                return AD_INVALID;
            }
            f->bytes[f->used++] = byte;
            f->state = 1;
        }
    }
    return AD_INCOMPLETE;
}

enum ad_status ad_frame_peek(const struct ad_framer *f,
                             const unsigned char **bytes, size_t *length)
{
    if (!f || !bytes || !length)
    {
        return AD_INVALID;
    }
    if (f->state != 2)
    {
        return AD_INCOMPLETE;
    }
    *bytes = f->bytes;
    *length = f->used;
    return AD_OK;
}

enum ad_status ad_frame_ack(struct ad_framer *f)
{
    if (!f)
    {
        return AD_INVALID;
    }
    if (f->state != 2)
    {
        return AD_INVALID;
    }
    f->state = f->used = f->expected = f->escaped = 0;
    return AD_OK;
}

enum ad_status ad_frame_end(struct ad_framer *f)
{
    if (!f)
    {
        return AD_INVALID;
    }
    if (f->state == 2)
    {
        return AD_READY;
    }
    if (f->state >= 3)
    {
        return AD_FAILED;
    }
    if (f->state == 1 || f->escaped)
    {
        f->state = 3;
        return AD_INCOMPLETE;
    }
    f->state = 4;
    return AD_OK;
}

void ad_frame_cancel(struct ad_framer *f)
{
    if (f)
    {
        for (unsigned i = 0; i < 8; ++i)
        {
            f->bytes[i] = 0;
        }
        f->used = f->expected = f->escaped = 0;
        f->state = 4;
    }
}

struct ad_nested
{
    const unsigned char *bytes;
    unsigned char leaves[32];
    size_t count, used, budget;
    unsigned max_depth;
};

static enum ad_status nested_sequence(struct ad_nested *s, size_t at,
                                      size_t end, unsigned depth)
{
    if (depth > s->max_depth)
    {
        return AD_SPACE;
    }
    while (at < end)
    {
        if (end - at < 2)
        {
            return AD_INCOMPLETE;
        }
        if (s->budget - s->used < 2)
        {
            return AD_SPACE;
        }
        unsigned tag = s->bytes[at++], length = s->bytes[at++];
        s->used += 2;
        if (tag == 0)
        {
            s->leaves[s->count++] = (unsigned char)length;
        }
        else if (tag == 1)
        {
            if (length > end - at)
            {
                return AD_INCOMPLETE;
            }
            enum ad_status rc = nested_sequence(s, at, at + length, depth + 1);
            if (rc != AD_OK)
            {
                return rc;
            }
            at += length;
        }
        else
        {
            return AD_INVALID;
        }
    }
    return AD_OK;
}

enum ad_status ad_nested_frames(const unsigned char *p, size_t n,
                                unsigned max_depth, size_t budget,
                                unsigned char *out, size_t capacity,
                                size_t *count)
{
    if ((n && !p) || n > 64 || max_depth > 3 || !count)
    {
        return AD_INVALID;
    }
    struct ad_nested state = {
        .bytes = p, .budget = budget, .max_depth = max_depth};
    enum ad_status rc = nested_sequence(&state, 0, n, 0);
    if (rc != AD_OK)
    {
        return rc;
    }
    if (state.count > capacity || (state.count && !out))
    {
        return AD_SPACE;
    }
    for (size_t i = 0; i < state.count; ++i)
    {
        out[i] = state.leaves[i];
    }
    *count = state.count;
    return AD_OK;
}
