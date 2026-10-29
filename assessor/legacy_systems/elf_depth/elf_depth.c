#include "elf_depth.h"

static uint64_t number(const unsigned char *p, unsigned n, unsigned order)
{
    uint64_t x = 0;
    for (unsigned i = 0; i < n; i++)
    {
        unsigned at = order == 1 ? n - 1 - i : i;
        x = (x << 8) | p[at];
    }
    return x;
}

static int extent(size_t length, uint64_t offset, uint64_t count, uint64_t size)
{
    if (offset > length || !size)
    {
        return 0;
    }
    if (!count)
    {
        return 1;
    }
    if (size > length || count > length)
    {
        return 0;
    }
    return (size_t)count <= (length - (size_t)offset) / (size_t)size;
}

static int align_ok(uint64_t a)
{
    return !a || !(a & (a - 1));
}

enum ed_status ed_section(const struct ed_view *v, unsigned i,
                          struct ed_section *out)
{
    if (!v || !out)
    {
        return ED_ARGUMENT;
    }
    if (i >= v->shnum)
    {
        return ED_RANGE;
    }
    const unsigned char *p =
        v->bytes + (size_t)v->shoff + (size_t)i * v->shentsize;
    unsigned w = v->bits / 8;
    struct ed_section s;
    s.name = (uint32_t)number(p, 4, v->order);
    s.type = (uint32_t)number(p + 4, 4, v->order);
    s.flags = number(p + 8, w, v->order);
    s.address = number(p + 8 + w, w, v->order);
    s.offset = number(p + 8 + 2 * w, w, v->order);
    s.size = number(p + 8 + 3 * w, w, v->order);
    s.link = (uint32_t)number(p + 8 + 4 * w, 4, v->order);
    s.info = (uint32_t)number(p + 12 + 4 * w, 4, v->order);
    s.align = number(p + 16 + 4 * w, w, v->order);
    s.entsize = number(p + 16 + 5 * w, w, v->order);
    *out = s;
    return ED_OK;
}

enum ed_status ed_program(const struct ed_view *v, unsigned i,
                          struct ed_program *out)
{
    if (!v || !out)
    {
        return ED_ARGUMENT;
    }
    if (i >= v->phnum)
    {
        return ED_RANGE;
    }
    const unsigned char *p =
        v->bytes + (size_t)v->phoff + (size_t)i * v->phentsize;
    unsigned w = v->bits / 8, b = v->bits == 64 ? 8 : 4;
    struct ed_program s;
    s.type = (uint32_t)number(p, 4, v->order);
    s.flags = (uint32_t)number(p + (v->bits == 64 ? 4 : 24), 4, v->order);
    s.offset = number(p + b, w, v->order);
    s.address = number(p + b + w, w, v->order);
    s.physical = number(p + b + 2 * w, w, v->order);
    s.filesz = number(p + b + 3 * w, w, v->order);
    s.memsz = number(p + b + 4 * w, w, v->order);
    s.align = number(p + (v->bits == 64 ? 48 : 28), w, v->order);
    *out = s;
    return ED_OK;
}

enum ed_status ed_symbol(const struct ed_view *v, unsigned table, size_t index,
                         struct ed_symbol *out)
{
    struct ed_section t;
    if (!v || !out)
    {
        return ED_ARGUMENT;
    }
    enum ed_status status = ed_section(v, table, &t);
    if (status != ED_OK)
    {
        return status;
    }
    if ((t.type != 2 && t.type != 11) || !t.entsize ||
        index >= (size_t)t.size / (size_t)t.entsize)
    {
        return ED_RANGE;
    }
    const unsigned char *p =
        v->bytes + (size_t)t.offset + index * (size_t)t.entsize;
    struct ed_symbol s;
    s.name = (uint32_t)number(p, 4, v->order);
    s.info = p[v->bits == 64 ? 4 : 12];
    s.other = p[v->bits == 64 ? 5 : 13];
    s.section = (unsigned)number(p + (v->bits == 64 ? 6 : 14), 2, v->order);
    s.value = number(p + (v->bits == 64 ? 8 : 4), v->bits / 8, v->order);
    s.size = number(p + (v->bits == 64 ? 16 : 8), v->bits / 8, v->order);
    *out = s;
    return ED_OK;
}

enum ed_status ed_name(const struct ed_view *v, unsigned table, uint32_t at,
                       const char **out, size_t *length)
{
    if (!v || !out || !length)
    {
        return ED_ARGUMENT;
    }
    struct ed_section s;
    enum ed_status status = ed_section(v, table, &s);
    if (status != ED_OK)
    {
        return status;
    }
    if (s.type != 3 || at >= s.size)
    {
        return ED_RANGE;
    }
    const unsigned char *p = v->bytes + (size_t)s.offset + at;
    size_t n = 0;
    while (n < (size_t)s.size - at && p[n])
    {
        if (n == 1024)
        {
            return ED_UNSUPPORTED;
        }
        n++;
    }
    if (n == (size_t)s.size - at)
    {
        return ED_FORMAT;
    }
    *out = (const char *)p;
    *length = n;
    return ED_OK;
}

enum ed_status ed_open(const unsigned char *p, size_t n, struct ed_view *out)
{
    if (!out || (!p && n))
    {
        return ED_ARGUMENT;
    }
    if (n > 16u * 1024u * 1024u)
    {
        return ED_UNSUPPORTED;
    }
    if (n < 16 || p[0] != 127 || p[1] != 'E' || p[2] != 'L' || p[3] != 'F')
    {
        return ED_FORMAT;
    }
    if ((p[4] != 1 && p[4] != 2) || (p[5] != 1 && p[5] != 2))
    {
        return ED_UNSUPPORTED;
    }
    unsigned bits = p[4] == 1 ? 32 : 64, w = bits / 8,
             hsize = bits == 64 ? 64 : 52, tail = bits == 64 ? 52 : 40;
    if (n < hsize || p[6] != 1 || number(p + 20, 4, p[5]) != 1 ||
        number(p + tail, 2, p[5]) != hsize)
    {
        return ED_FORMAT;
    }
    struct ed_view v = {p,
                        n,
                        bits,
                        p[5],
                        (unsigned)number(p + 16, 2, p[5]),
                        (unsigned)number(p + 18, 2, p[5]),
                        number(p + 24, w, p[5]),
                        number(p + 24 + 2 * w, w, p[5]),
                        number(p + 24 + w, w, p[5]),
                        (unsigned)number(p + tail + 8, 2, p[5]),
                        (unsigned)number(p + tail + 4, 2, p[5]),
                        (unsigned)number(p + tail + 10, 2, p[5]),
                        (unsigned)number(p + tail + 6, 2, p[5]),
                        (unsigned)number(p + tail + 2, 2, p[5])};
    if (v.phnum == 65535 || v.shstr == 65535 || (!v.shnum && v.shoff) ||
        v.shnum > 4096 || v.phnum > 4096)
    {
        return ED_UNSUPPORTED;
    }
    if ((v.shnum && (v.shentsize != (bits == 64 ? 64u : 40u) ||
                     !extent(n, v.shoff, v.shnum, v.shentsize))) ||
        (v.phnum && (v.phentsize != (bits == 64 ? 56u : 32u) ||
                     !extent(n, v.phoff, v.phnum, v.phentsize))))
    {
        return ED_RANGE;
    }
    if (v.shstr && (v.shstr >= v.shnum))
    {
        return ED_RANGE;
    }
    for (unsigned i = 0; i < v.shnum; i++)
    {
        struct ed_section s;
        ed_section(&v, i, &s);
        if (!s.type)
        {
            continue;
        }
        if (!align_ok(s.align) || (s.align && (s.address & (s.align - 1))))
        {
            return ED_FORMAT;
        }
        if (s.type != 8 && !extent(n, s.offset, s.size, 1))
        {
            return ED_RANGE;
        }
        if (s.type == 3 && s.size &&
            (p[(size_t)s.offset] || p[(size_t)(s.offset + s.size - 1)]))
        {
            return ED_FORMAT;
        }
    }
    size_t symbols = 0, name_work = 0;
    for (unsigned i = 0; i < v.shnum; i++)
    {
        struct ed_section s;
        ed_section(&v, i, &s);
        if (!s.type)
        {
            continue;
        }
        const char *name;
        size_t length;
        if (s.name)
        {
            if (!v.shstr)
            {
                return ED_FORMAT;
            }
            enum ed_status status =
                ed_name(&v, v.shstr, s.name, &name, &length);
            if (status != ED_OK)
            {
                return status;
            }
            name_work += length + 1;
        }
        if (s.type == 2 || s.type == 11)
        {
            struct ed_section strings;
            if (s.entsize != (bits == 64 ? 24u : 16u))
            {
                return ED_FORMAT;
            }
            size_t count = (size_t)s.size / (size_t)s.entsize;
            if ((size_t)s.size % (size_t)s.entsize || s.info > count ||
                ed_section(&v, s.link, &strings) != ED_OK || strings.type != 3)
            {
                return ED_FORMAT;
            }
            if (count > 16384 - symbols)
            {
                return ED_UNSUPPORTED;
            }
            symbols += count;
            for (size_t j = 0; j < count; j++)
            {
                struct ed_symbol symbol;
                ed_symbol(&v, i, j, &symbol);
                if (symbol.section == 65535)
                {
                    return ED_UNSUPPORTED;
                }
                if (symbol.section < 0xff00 && symbol.section >= v.shnum)
                {
                    return ED_RANGE;
                }
                if (symbol.name)
                {
                    enum ed_status status =
                        ed_name(&v, s.link, symbol.name, &name, &length);
                    if (status != ED_OK)
                    {
                        return status;
                    }
                    name_work += length + 1;
                }
                if (name_work > 8u * 1024u * 1024u)
                {
                    return ED_UNSUPPORTED;
                }
            }
        }
    }
    uint64_t previous = 0;
    unsigned loads = 0;
    for (unsigned i = 0; i < v.phnum; i++)
    {
        struct ed_program s;
        ed_program(&v, i, &s);
        if (!s.type)
        {
            continue;
        }
        if (!extent(n, s.offset, s.filesz, 1))
        {
            return ED_RANGE;
        }
        if (s.type == 1)
        {
#ifndef ED_MUTANT_SEGMENT
            uint64_t address_max = v.bits == 32 ? UINT32_MAX : UINT64_MAX;
            if (s.filesz > s.memsz || s.memsz > address_max - s.address)
            {
                return ED_RANGE;
            }
#endif
            if (!align_ok(s.align) ||
                (s.align > 1 && ((s.address ^ s.offset) & (s.align - 1))) ||
                (loads && s.address < previous))
            {
                return ED_FORMAT;
            }
            previous = s.address;
            loads++;
        }
    }
    /* Publish fields only after validation. Avoid a compiler-generated aggregate
     * copy helper on the strict target; the object audit must confirm this. */
    out->bytes = v.bytes;
    out->length = v.length;
    out->bits = v.bits;
    out->order = v.order;
    out->type = v.type;
    out->machine = v.machine;
    out->entry = v.entry;
    out->shoff = v.shoff;
    out->phoff = v.phoff;
    out->shnum = v.shnum;
    out->phnum = v.phnum;
    out->shstr = v.shstr;
    out->shentsize = v.shentsize;
    out->phentsize = v.phentsize;
    return ED_OK;
}
