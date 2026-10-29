#include "wire.h"
#include <string.h>
#ifndef EW_MUTANT
#define EW_MUTANT 0
#endif
int ew_feed(struct ew_stream *s, const unsigned char *b, size_t n, size_t *used)
{
    size_t i = 0;
    if (!s || !used || (!b && n) || s->phase > 4 || s->prefix > 2)
    {
        return EW_INVALID;
    }
    if (s->phase == 2)
    {
        *used = 0;
        return EW_READY;
    }
    if (s->phase >= 3)
    {
        *used = 0;
        return s->phase == 3 ? EW_POISON : EW_EOF;
    }
    if (s->used > s->length || s->length > 256)
    {
        return EW_INVALID;
    }
    while (i < n)
    {
        if (s->prefix < 2)
        {
            s->length = (EW_MUTANT == 6
                             ? s->length | ((unsigned)b[i] << (8 * s->prefix))
                             : (s->length << 8) | b[i]);
            ++i;
            ++s->prefix;
            if (s->prefix == 2)
            {
                if (s->length > 256)
                {
                    s->phase = 3;
                    *used = i;
                    return EW_POISON;
                }
                if (!s->length)
                {
                    s->phase = 2;
                    break;
                }
            }
        }
        else
        {
            s->data[s->used++] = b[i++];
            if (s->used == s->length)
            {
                s->phase = 2;
                break;
            }
        }
    }
    if (EW_MUTANT == 1 && i < n)
    {
        i = n;
    }
    *used = i;
    return s->phase == 2 ? EW_READY : EW_MORE;
}

int ew_take(struct ew_stream *s, unsigned char *b, size_t cap, size_t *n)
{
    if (!s || !n || (!b && cap) || s->phase > 4)
    {
        return EW_INVALID;
    }
    if (s->phase != 2)
    {
        return EW_MORE;
    }
    if (cap < s->length)
    {
        return EW_FULL;
    }
    if (s->length)
    {
        memcpy(b, s->data, s->length);
    }
    *n = s->length;
    memset(s, 0, sizeof *s);
    return EW_OK;
}

int ew_end(struct ew_stream *s)
{
    if (!s || s->phase > 4)
    {
        return EW_INVALID;
    }
    if (s->phase == 2)
    {
        return EW_READY;
    }
    if (s->phase == 3)
    {
        return EW_POISON;
    }
    if (s->phase == 4)
    {
        return EW_EOF;
    }
    if (s->prefix || s->used)
    {
        s->phase = 3;
        return EW_TRUNCATED;
    }
    s->phase = 4;
    return EW_EOF;
}

static unsigned u16(const unsigned char *p)
{
    return (unsigned)p[0] * 256 + p[1];
}

static void p16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static void fcs(unsigned char *p, size_t n)
{
    uint32_t c = 0;
    unsigned i;
    (void)ed_crc32(p, n, &c);
    for (i = 0; i < 4; ++i)
    {
        p[n + i] = (unsigned char)(c >> (8 * i));
    }
}

int ew_vlan_build(const unsigned char *d, const unsigned char *s, unsigned pri,
                  unsigned vid, unsigned field, const unsigned char *b,
                  size_t n, unsigned char *out, size_t cap, size_t *written)
{
    unsigned char t[1522];
    size_t stored = n < 42 ? 42 : n, total = 18 + stored + 4;
    if (!d || !s || !out || !written || (!b && n) || n > 1500 || pri > 7 ||
        !vid || vid > 4094 || field > 65535)
    {
        return EW_INVALID;
    }
    if ((field <= 1500 && field != n) || (field > 1500 && field < 1536) ||
        field == 0x8100 || field == 0x88a8)
    {
        return EW_INVALID;
    }
    if (cap < total)
    {
        return EW_FULL;
    }
    memcpy(t, d, 6);
    memcpy(t + 6, s, 6);
    p16(t + 12, 0x8100);
    p16(t + 14, (pri << 13) | vid);
    p16(t + 16, field);
    if (n)
    {
        memcpy(t + 18, b, n);
    }
    memset(t + 18 + n, 0, stored - n);
    fcs(t, total - 4);
    if (EW_MUTANT == 2)
    {
        t[14] |= 0x10;
    }
    memcpy(out, t, total);
    *written = total;
    return EW_OK;
}

int ew_vlan_parse(const unsigned char *b, size_t n, struct ew_vlan *out)
{
    struct ew_vlan t;
    uint32_t c = 0, got = 0;
    unsigned i, field, tag;
    size_t stored;
    if (!b || !out || n < 64 || n > 1522 || u16(b + 12) != 0x8100)
    {
        return EW_INVALID;
    }
    tag = u16(b + 14);
    field = u16(b + 16);
    stored = n - 22;
    if ((tag & 0x1000) || !(tag & 4095) || (tag & 4095) == 4095 ||
        (field > 1500 && field < 1536) || field == 0x8100 || field == 0x88a8 ||
        (field <= 1500 && field > stored))
    {
        return EW_INVALID;
    }
    (void)ed_crc32(b, n - 4, &c);
    for (i = 0; i < 4; ++i)
    {
        got |= (uint32_t)b[n - 4 + i] << (8 * i);
    }
    if (EW_MUTANT != 3 && c != got)
    {
        return EW_POISON;
    }
    memset(&t, 0, sizeof t);
    memcpy(t.frame.destination, b, 6);
    memcpy(t.frame.source, b + 6, 6);
    t.frame.field = (uint16_t)field;
    t.frame.payload = b + 18;
    t.frame.stored_payload = stored;
    t.frame.payload_length = field <= 1500 ? field : stored;
    t.priority = tag >> 13;
    t.vid = tag & 4095;
    *out = t;
    return EW_OK;
}

int ew_vlan_pop(const unsigned char *b, size_t n, unsigned char *out,
                size_t cap, size_t *written)
{
    struct ew_vlan v;
    int r = ew_vlan_parse(b, n, &v);
    if (r != EW_OK)
    {
        return r;
    }
    r = ed_frame_build(v.frame.destination, v.frame.source, v.frame.field,
                       v.frame.payload, v.frame.payload_length, 1, out, cap,
                       written);
    return r == ED_OK ? EW_OK : r == ED_FULL ? EW_FULL : EW_INVALID;
}

static int equal(const unsigned char *a, const unsigned char *b)
{
    unsigned i;
    for (i = 0; i < 6; ++i)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }
    return 1;
}

int ew_bridge(struct ed_bridge *s, const unsigned char *src,
              const unsigned char *dst, unsigned port, uint32_t now,
              unsigned evict, unsigned *mask)
{
    struct ed_bridge t;
    unsigned i, j, victim = 0;
    int r;
    if (!s || !src || !dst || !mask || evict > 1)
    {
        return ED_INVALID;
    }
    for (i = 0; i < 4; ++i)
    {
        if (s->entries[i].live > 1 ||
            (s->entries[i].live && (s->entries[i].mac[0] & 1)))
        {
            return ED_INVALID;
        }
    }
    for (i = 0; i < 4; ++i)
    {
        for (j = i + 1; j < 4; ++j)
        {
            if (s->entries[i].live && s->entries[j].live &&
                equal(s->entries[i].mac, s->entries[j].mac))
            {
                return ED_INVALID;
            }
        }
    }
    t = *s;
    r = ed_bridge(&t, src, dst, port, now, mask);
    if (r == ED_FULL && evict)
    {
        for (i = 1; i < 4; ++i)
        {
            if (EW_MUTANT == 5 ? t.entries[i].seen > t.entries[victim].seen
                               : t.entries[i].seen < t.entries[victim].seen)
            {
                victim = i;
            }
        }
        t.entries[victim].live = 0;
        r = ed_bridge(&t, src, dst, port, now, mask);
    }
    if (r == ED_OK)
    {
        if (EW_MUTANT == 4)
        {
            *mask |= 1u << (port - 1);
        }
        *s = t;
    }
    return r;
}
