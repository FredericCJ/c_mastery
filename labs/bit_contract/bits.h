#ifndef FB_BITS_H
#define FB_BITS_H
#include <stddef.h>
#include <stdint.h>
typedef uint_least32_t fb_word;

enum fb_status
{
    FB_OK,
    FB_INVALID,
    FB_RANGE,
    FB_SHORT,
    FB_UNSUPPORTED,
    FB_CONFLICT,
    FB_SKIPPED,
    FB_UNFINISHED
};

enum fb_shift
{
    FB_LEFT,
    FB_RIGHT,
    FB_ROTATE_LEFT
};

enum fb_patch
{
    FB_SEQUENTIAL,
    FB_SIMULTANEOUS
};

enum fb_order
{
    FB_BIG,
    FB_LITTLE
};

enum fb_unknown
{
    FB_REJECT_UNKNOWN,
    FB_SKIP_UNKNOWN
};

struct fb_edit
{
    unsigned low, high;
    fb_word value;
};

struct fb_stats
{
    unsigned population, width, leading;
};

struct fb_span
{
    const unsigned char *data;
    size_t size;
};

struct fb_record
{
    fb_word value;
    unsigned flags;
    size_t consumed;
};

struct fb_plugin
{
    unsigned version;
    size_t size;
    fb_word features, max_items;
};

struct fb_register
{
    unsigned char state;
    unsigned reads, writes;
    unsigned read_clear;
};
enum fb_status fb_shift(unsigned width, fb_word input, int count,
                        enum fb_shift policy, fb_word *out);
enum fb_status fb_extract(unsigned width, fb_word input, unsigned low,
                          unsigned high, fb_word *out);
enum fb_status fb_insert(unsigned width, fb_word input, struct fb_edit edit,
                         fb_word *out);
enum fb_status fb_patch(unsigned width, fb_word input, struct fb_edit first,
                        struct fb_edit second, enum fb_patch policy,
                        fb_word *out);
enum fb_status fb_stats(unsigned width, fb_word input, struct fb_stats *out);
enum fb_status fb_encode(fb_word value, enum fb_order order, unsigned char *out,
                         size_t capacity);
enum fb_status fb_decode(struct fb_span input, enum fb_order order,
                         fb_word *out);
enum fb_status fb_decode_two(struct fb_span first, struct fb_span second,
                             enum fb_order order, fb_word *out);
enum fb_status fb_record(struct fb_span input, enum fb_unknown policy,
                         struct fb_record *out);
enum fb_status fb_negotiate(struct fb_span input, struct fb_plugin *out);
/* Original teaching register: low4 ordinary, high4 W1C; read_clear optionally
 * clears high4 after returning old state. No volatile/MMIO or concurrency claim. */
enum fb_status fb_register_init(struct fb_register *reg, unsigned state,
                                unsigned read_clear);
unsigned fb_register_read(struct fb_register *reg);
void fb_register_write(struct fb_register *reg, unsigned value);
enum fb_status fb_register_update(struct fb_register *reg, unsigned low_value,
                                  unsigned clear_status);
#endif
