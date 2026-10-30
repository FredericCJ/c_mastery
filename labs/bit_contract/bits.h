/*
 * Bit, codec and register contracts.
 *
 * Common rules for every function below:
 *  - fb_word holds a logical value of 1..16 bits (`width`); bits above the
 *    width are ignored on input and zero on output.
 *  - Bit ranges are half-open: [low, high) selects bits low .. high-1, so
 *    low == high is the empty field and high == width is the top bit.
 *  - Output pointers must be non-NULL, live and writable. Failure atomicity:
 *    on any return other than FB_OK (FB_SKIPPED excepted, see fb_record) the
 *    caller's outputs are unchanged.
 *  - Spans are borrowed for one call and never retained or freed. A span whose
 *    data is NULL is valid only when its size is 0.
 *  - Nothing here allocates or blocks.
 *  - Status codes: FB_INVALID for a bad argument or enumeration value,
 *    FB_RANGE when a count, field or value is outside what the width or format
 *    allows, FB_SHORT when fewer octets are available than a format needs,
 *    FB_UNSUPPORTED for an unknown version or when CHAR_BIT != 8,
 *    FB_CONFLICT for overlapping simultaneous edits that disagree,
 *    FB_SKIPPED for a record deliberately skipped, FB_UNFINISHED only from the
 *    learner scaffold.
 */
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

/* FB_SEQUENTIAL: the second edit overwrites the first where they overlap.
 * FB_SIMULTANEOUS: overlapping bits must agree, else FB_CONFLICT. */
enum fb_patch
{
    FB_SEQUENTIAL,
    FB_SIMULTANEOUS
};

/* Octet order on the wire: most significant octet first (FB_BIG) or last. */
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

/* Write `value` into bits [low, high); value must fit in high - low bits. */
struct fb_edit
{
    unsigned low;
    unsigned high;
    fb_word value;
};

/* Bit statistics of a width-bit value. population: number of set bits.
 * width: index of the highest set bit plus one (0 for zero). leading: width of
 * the field minus that, i.e. the count of leading zero bits within the field
 * (the field width itself for zero). */
struct fb_stats
{
    unsigned population;
    unsigned width;
    unsigned leading;
};

struct fb_span
{
    const unsigned char *data;
    size_t size; /* octets */
};

struct fb_record
{
    fb_word value;
    unsigned flags;
    size_t consumed; /* octets the record occupies, from its declared size */
};

struct fb_plugin
{
    unsigned version;
    size_t size; /* octets */
    fb_word features;
    fb_word max_items;
};

struct fb_register
{
    unsigned char state;
    unsigned reads;
    unsigned writes;
    unsigned read_clear;
};

/* Shift or rotate the low `width` bits. count must satisfy 0 <= count < width
 * (FB_RANGE otherwise); width must be 1..16 (FB_INVALID otherwise). The result
 * is masked to the width. */
enum fb_status fb_shift(unsigned width, fb_word input, int count,
                        enum fb_shift policy, fb_word *out);

/* Bits [low, high) of input, right-aligned. FB_RANGE unless low <= high <=
 * width. */
enum fb_status fb_extract(unsigned width, fb_word input, unsigned low,
                          unsigned high, fb_word *out);

/* Replace bits [edit.low, edit.high) of input with edit.value. FB_RANGE for a
 * bad range or a value that does not fit the field. */
enum fb_status fb_insert(unsigned width, fb_word input, struct fb_edit edit,
                         fb_word *out);

/* Apply two edits in order. With FB_SIMULTANEOUS, overlapping bits that the
 * edits set to different values return FB_CONFLICT and *out is unchanged. */
enum fb_status fb_patch(unsigned width, fb_word input, struct fb_edit first,
                        struct fb_edit second, enum fb_patch policy,
                        fb_word *out);

enum fb_status fb_stats(unsigned width, fb_word input, struct fb_stats *out);

/* Write value as four octets in the given order to out[0..3]. capacity is in
 * octets; FB_SHORT if it is below 4. FB_RANGE if value does not fit in 32
 * bits. Octets are written only on FB_OK. */
enum fb_status fb_encode(fb_word value, enum fb_order order, unsigned char *out,
                         size_t capacity);

/* Read four octets from the span; FB_SHORT if fewer are available. Octets
 * beyond the fourth are not read. */
enum fb_status fb_decode(struct fb_span input, enum fb_order order,
                         fb_word *out);

/* As fb_decode, but the four octets may straddle two spans (either may be
 * empty). Reads only octets that exist in the spans. */
enum fb_status fb_decode_two(struct fb_span first, struct fb_span second,
                             enum fb_order order, fb_word *out);

/* Parse one record: octet 0 version, octet 1 flags, octets 2..3 big-endian
 * total size, then the payload. Only version 1 with no unknown flags and size 8
 * is decoded. An unknown version or flag returns FB_UNSUPPORTED under
 * FB_REJECT_UNKNOWN (out unchanged), or FB_SKIPPED under FB_SKIP_UNKNOWN with
 * out = {0, 0, declared size} so the caller can step over it; its payload is
 * never read. FB_SHORT if fewer than 4 octets are available or the declared
 * size exceeds the available octets. FB_RANGE if the declared size is below 4,
 * or if a version-1 record without unknown flags declares a size other
 * than 8. */
enum fb_status fb_record(struct fb_span input, enum fb_unknown policy,
                         struct fb_record *out);

/* Parse a plugin descriptor: octets 0..1 big-endian version, 2..3 total size.
 * Versions 1 (8 octets) and 2 (12 octets) are understood; the declared size
 * must fit in the available octets and cover every field of the version.
 * Unknown versions or feature bits return FB_UNSUPPORTED; no field beyond the
 * declared size is read. */
enum fb_status fb_negotiate(struct fb_span input, struct fb_plugin *out);

/* Original teaching register: low4 ordinary, high4 W1C; read_clear optionally
 * clears high4 after returning old state. No volatile/MMIO or concurrency
 * claim.
 * fb_register_init: FB_INVALID if reg is NULL, state > 255 or read_clear > 1.
 * The other register functions require a non-NULL reg initialised by it.
 * fb_register_update writes the low nibble and clears the named status bits in
 * one write, without reading; FB_INVALID if low_value > 15, clear_status names
 * non-status bits or exceeds 240. */
enum fb_status fb_register_init(struct fb_register *reg, unsigned state,
                                unsigned read_clear);
unsigned fb_register_read(struct fb_register *reg);
void fb_register_write(struct fb_register *reg, unsigned value);
enum fb_status fb_register_update(struct fb_register *reg, unsigned low_value,
                                  unsigned clear_status);
#endif
