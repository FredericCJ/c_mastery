#include "bits.h"

enum fb_status fb_shift(unsigned width, fb_word input, int count,
                        enum fb_shift policy, fb_word *out)
{
    (void)width;
    (void)input;
    (void)count;
    (void)policy;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_extract(unsigned width, fb_word input, unsigned low,
                          unsigned high, fb_word *out)
{
    (void)width;
    (void)input;
    (void)low;
    (void)high;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_insert(unsigned width, fb_word input, struct fb_edit edit,
                         fb_word *out)
{
    (void)width;
    (void)input;
    (void)edit;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_patch(unsigned width, fb_word input, struct fb_edit first,
                        struct fb_edit second, enum fb_patch policy,
                        fb_word *out)
{
    (void)width;
    (void)input;
    (void)first;
    (void)second;
    (void)policy;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_stats(unsigned width, fb_word input, struct fb_stats *out)
{
    (void)width;
    (void)input;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_encode(fb_word value, enum fb_order order, unsigned char *out,
                         size_t capacity)
{
    (void)value;
    (void)order;
    (void)out;
    (void)capacity;
    return FB_UNFINISHED;
}

enum fb_status fb_decode(struct fb_span input, enum fb_order order,
                         fb_word *out)
{
    (void)input;
    (void)order;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_decode_two(struct fb_span first, struct fb_span second,
                             enum fb_order order, fb_word *out)
{
    (void)first;
    (void)second;
    (void)order;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_record(struct fb_span input, enum fb_unknown policy,
                         struct fb_record *out)
{
    (void)input;
    (void)policy;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_negotiate(struct fb_span input, struct fb_plugin *out)
{
    (void)input;
    (void)out;
    return FB_UNFINISHED;
}

enum fb_status fb_register_init(struct fb_register *reg, unsigned state,
                                unsigned read_clear)
{
    (void)reg;
    (void)state;
    (void)read_clear;
    return FB_UNFINISHED;
}

unsigned fb_register_read(struct fb_register *reg)
{
    (void)reg;
    return 0;
}

void fb_register_write(struct fb_register *reg, unsigned value)
{
    (void)reg;
    (void)value;
}

enum fb_status fb_register_update(struct fb_register *reg, unsigned low_value,
                                  unsigned clear_status)
{
    (void)reg;
    (void)low_value;
    (void)clear_status;
    return FB_UNFINISHED;
}
