#include "bits.h"
#include "integer.h" /* Actual prerequisite cell codec. */
#include <limits.h>
#ifndef FB_MUTANT
#define FB_MUTANT 0
#endif
static int width_ok(unsigned width)
{
    return width > 0 && width <= 16;
}

static fb_word mask(unsigned width)
{
    return ((fb_word)1 << width) - 1;
}

enum fb_status fb_shift(unsigned width, fb_word input, int count,
                        enum fb_shift policy, fb_word *out)
{
    if (!out || !width_ok(width) || (unsigned)policy > FB_ROTATE_LEFT)
        return FB_INVALID;
    if (count < 0 || (unsigned)count >= width)
        return FB_RANGE;
    fb_word value = FB_MUTANT == 1 ? input : input & mask(width), result;
    if (policy == FB_LEFT)
        result = value << count;
    else if (policy == FB_RIGHT)
        result = value >> count;
    else
        result = count ? (value << count) | (value >> (width - (unsigned)count))
                       : value;
    *out = result & mask(width);
    return FB_OK;
}

enum fb_status fb_extract(unsigned width, fb_word input, unsigned low,
                          unsigned high, fb_word *out)
{
    if (!out || !width_ok(width))
        return FB_INVALID;
    if (low > high || high > width)
        return FB_RANGE;
    *out = (input >> low) & mask(high - low);
    return FB_OK;
}

enum fb_status fb_insert(unsigned width, fb_word input, struct fb_edit edit,
                         fb_word *out)
{
    if (!out || !width_ok(width))
        return FB_INVALID;
    if (edit.low > edit.high || edit.high > width)
        return FB_RANGE;
    fb_word field = mask(edit.high - edit.low);
    if (edit.value > field)
        return FB_RANGE;
    *out = ((input & ~(field << edit.low)) | (edit.value << edit.low)) &
           mask(width);
    return FB_OK;
}

enum fb_status fb_patch(unsigned width, fb_word input, struct fb_edit a,
                        struct fb_edit b, enum fb_patch policy, fb_word *out)
{
    if (!out || (unsigned)policy > FB_SIMULTANEOUS)
        return FB_INVALID;
    fb_word first, second;
    enum fb_status s = fb_insert(width, input, a, &first);
    if (s != FB_OK)
        return s;
    s = fb_insert(width, first, b, &second);
    if (s != FB_OK)
        return s;
    if (policy == FB_SIMULTANEOUS && FB_MUTANT != 2)
    {
        fb_word overlap =
            (mask(a.high - a.low) << a.low) & (mask(b.high - b.low) << b.low);
        if (((a.value << a.low) ^ (b.value << b.low)) & overlap)
            return FB_CONFLICT;
    }
    *out = second;
    return FB_OK;
}

enum fb_status fb_stats(unsigned width, fb_word input, struct fb_stats *out)
{
    if (!out || !width_ok(width))
        return FB_INVALID;
    fb_word value = input & mask(width);
    struct fb_stats s = {0, 0, width};
    for (unsigned i = 0; i < width; ++i)
        if ((value >> i) & 1)
        {
            ++s.population;
            s.width = i + 1;
        }
    s.leading = width - s.width;
    if (FB_MUTANT == 3 && !value)
        s.leading = 0;
    *out = s;
    return FB_OK;
}

enum fb_status fb_encode(fb_word value, enum fb_order order, unsigned char *out,
                         size_t capacity)
{
    if (!out || (unsigned)order > FB_LITTLE)
        return FB_INVALID;
    if (CHAR_BIT != 8)
        return FB_UNSUPPORTED;
    if (capacity < 4)
        return FB_SHORT;
    unsigned int cells[4];
    if (fi_encode_cells(value, cells) != FI_OK)
        return FB_RANGE;
    for (unsigned i = 0; i < 4; ++i)
        out[i] = (unsigned char)cells[order == FB_BIG ? i : 3 - i];
    return FB_OK;
}

static int span_ok(struct fb_span s)
{
    return s.data || !s.size;
}

enum fb_status fb_decode_two(struct fb_span a, struct fb_span b,
                             enum fb_order order, fb_word *out)
{
    if (!out || !span_ok(a) || !span_ok(b) || (unsigned)order > FB_LITTLE)
        return FB_INVALID;
    if (CHAR_BIT != 8)
        return FB_UNSUPPORTED;
    if (FB_MUTANT == 4)
        *out = 0;
    if (a.size < 4 && b.size < 4 - a.size)
        return FB_SHORT;
    unsigned int cells[4];
    for (unsigned i = 0; i < 4; ++i)
    {
        unsigned char v = i < a.size ? a.data[i] : b.data[i - a.size];
        cells[order == FB_BIG ? i : 3 - i] = v;
    }
    fb_word value;
    if (fi_decode_cells(cells, &value) != FI_OK)
        return FB_RANGE;
    *out = value;
    return FB_OK;
}

enum fb_status fb_decode(struct fb_span input, enum fb_order order,
                         fb_word *out)
{
    return fb_decode_two(input, (struct fb_span){NULL, 0}, order, out);
}

static unsigned u16(const unsigned char *p)
{
    return (unsigned)p[0] * 256u + p[1];
}

enum fb_status fb_record(struct fb_span input, enum fb_unknown policy,
                         struct fb_record *out)
{
    if (!out || !span_ok(input) || (unsigned)policy > FB_SKIP_UNKNOWN)
        return FB_INVALID;
    if (CHAR_BIT != 8)
        return FB_UNSUPPORTED;
    if (input.size < 4)
        return FB_SHORT;
    unsigned version = input.data[0], flags = input.data[1],
             size = u16(input.data + 2);
    if (size < 4)
        return FB_RANGE;
    if (size > input.size)
        return FB_SHORT;
    if (version != 1 || (flags & ~1u))
    {
        if (policy == FB_REJECT_UNKNOWN)
            return FB_UNSUPPORTED;
        *out = (struct fb_record){0, 0, size};
        return FB_SKIPPED;
    }
    if (size != 8)
        return FB_RANGE;
    fb_word value;
    enum fb_status s =
        fb_decode((struct fb_span){input.data + 4, 4}, FB_BIG, &value);
    if (s != FB_OK)
        return s;
    *out = (struct fb_record){value, flags, size};
    return FB_OK;
}

enum fb_status fb_negotiate(struct fb_span input, struct fb_plugin *out)
{
    if (!out || !span_ok(input))
        return FB_INVALID;
    if (CHAR_BIT != 8)
        return FB_UNSUPPORTED;
    if (input.size < 4)
        return FB_SHORT;
    unsigned version = u16(input.data), size = u16(input.data + 2);
    if (size < 4)
        return FB_RANGE;
    if (size > input.size)
        return FB_SHORT;
    if (version != 1 && version != 2)
        return FB_UNSUPPORTED;
    if (size < (version == 1 ? 8u : 12u))
        return FB_SHORT;
    fb_word features = 0, items = 0;
    enum fb_status s =
        fb_decode((struct fb_span){input.data + 4, 4}, FB_BIG, &features);
    if (s != FB_OK)
        return s;
    if (features & ~(fb_word)1)
        return FB_UNSUPPORTED;
    if (version == 2)
    {
        s = fb_decode((struct fb_span){input.data + 8, 4}, FB_BIG, &items);
        if (s != FB_OK)
            return s;
    }
    if (FB_MUTANT == 6 && version == 1)
        items = 999;
    *out = (struct fb_plugin){version, size, features, items};
    return FB_OK;
}

enum fb_status fb_register_init(struct fb_register *r, unsigned value,
                                unsigned read_clear)
{
    if (!r || value > 255 || read_clear > 1)
        return FB_INVALID;
    *r = (struct fb_register){(unsigned char)value, 0, 0, read_clear};
    return FB_OK;
}

unsigned fb_register_read(struct fb_register *r)
{
    unsigned value = r->state;
    ++r->reads;
    if (r->read_clear)
        r->state &= 15;
    return value;
}

void fb_register_write(struct fb_register *r, unsigned v)
{
    ++r->writes;
    r->state = (unsigned char)((v & 15) | ((r->state & 240) & ~(v & 240)));
}

enum fb_status fb_register_update(struct fb_register *r, unsigned low,
                                  unsigned clear)
{
    if (!r || low > 15 || clear > 240 || (clear & 15))
        return FB_INVALID;
    if (FB_MUTANT == 5)
    {
        unsigned value = fb_register_read(r);
        fb_register_write(r, (value & 240) | low | clear);
    }
    else
        fb_register_write(r, low | clear);
    return FB_OK;
}
