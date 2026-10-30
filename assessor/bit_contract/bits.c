/*
 * Reference implementation of the bit, codec and register contracts in bits.h.
 *
 * All bit manipulation is on fb_word (uint_least32_t, at least 32 bits) with
 * logical widths of 1..16 bits, so every shift count used below is smaller
 * than the width of the type (SEI CERT INT34-C) and no signed operand is
 * shifted (CERT INT13-C). Functions validate first and write their outputs
 * only on success.
 *
 * FB_MUTANT selects a deliberate defect for the mutation campaign; 0 builds
 * the correct reference.
 */
#include "bits.h"
#include "integer.h" /* Actual prerequisite cell codec. */
#include <limits.h>
#include <stdbool.h>

#ifndef FB_MUTANT
#define FB_MUTANT 0
#endif

#define MAX_LOGICAL_WIDTH_BITS 16u

/* Wire format shared by records and plugin descriptors: a 4-octet header whose
 * last two octets are the big-endian total size in octets. */
#define HEADER_OCTETS 4u
#define HEADER_SIZE_OFFSET 2u
#define WIRE_WORD_OCTETS 4u
#define OCTET_RADIX 256u

#define RECORD_VERSION 1u
#define RECORD_KNOWN_FLAGS 1u
#define RECORD_TOTAL_OCTETS 8u

#define PLUGIN_V1_OCTETS 8u
#define PLUGIN_V2_OCTETS 12u
#define PLUGIN_KNOWN_FEATURES 1u
#define PLUGIN_FEATURES_OFFSET 4u
#define PLUGIN_MAX_ITEMS_OFFSET 8u

/* Teaching register: low nibble is ordinary read/write, high nibble is
 * write-1-to-clear status (W1C). */
#define REGISTER_ORDINARY_MASK 0x0fu
#define REGISTER_STATUS_MASK 0xf0u
#define REGISTER_VALUE_MAX 0xffu

static bool width_is_supported(unsigned width)
{
    return (width > 0u) && (width <= MAX_LOGICAL_WIDTH_BITS);
}

/* Low `width` bits set. Requires width <= MAX_LOGICAL_WIDTH_BITS, so the shift
 * count is less than the width of fb_word. */
static fb_word mask(unsigned width)
{
    return ((fb_word)1 << width) - 1u;
}

/* Rotate the low `width` bits of value left by count, 0 <= count < width.
 * For a value already masked to `width` bits, count 0 gives (value << 0) |
 * (value >> width) == value, so no special case is needed. The caller still
 * handles count 0 itself so that a value carrying bits above `width` (mutant
 * 1, which skips masking) cannot wrap them into the low bits. The result may
 * carry bits above `width`; the caller masks them off. */
static fb_word rotate_left(fb_word value, unsigned count, unsigned width)
{
    return (value << count) | (value >> (width - count));
}

enum fb_status fb_shift(unsigned width, fb_word input, int count,
                        enum fb_shift policy, fb_word *out)
{
    if ((out == NULL) || !width_is_supported(width) ||
        ((unsigned)policy > (unsigned)FB_ROTATE_LEFT))
    {
        return FB_INVALID;
    }
    /* Negative counts are rejected before the conversion to unsigned. A count
     * equal to the width is a range error, not a silent "shift out
     * everything": the C operators have no such rule. */
    if ((count < 0) || ((unsigned)count >= width))
    {
        return FB_RANGE;
    }
    unsigned shift = (unsigned)count;

    /* Mutant 1 skips masking the input to its logical width. */
    fb_word value = (FB_MUTANT == 1) ? input : (input & mask(width));
    fb_word result = 0;
    if (policy == FB_LEFT)
    {
        result = value << shift;
    }
    else if (policy == FB_RIGHT)
    {
        result = value >> shift;
    }
    else
    {
        result = (shift != 0u) ? rotate_left(value, shift, width) : value;
    }
    *out = result & mask(width);
    return FB_OK;
}

/* Half-open bit range [low, high) inside a word of the given width. */
static bool field_fits(unsigned width, unsigned low, unsigned high)
{
    return (low <= high) && (high <= width);
}

enum fb_status fb_extract(unsigned width, fb_word input, unsigned low,
                          unsigned high, fb_word *out)
{
    if ((out == NULL) || !width_is_supported(width))
    {
        return FB_INVALID;
    }
    if (!field_fits(width, low, high))
    {
        return FB_RANGE;
    }
    *out = (input >> low) & mask(high - low);
    return FB_OK;
}

enum fb_status fb_insert(unsigned width, fb_word input, struct fb_edit edit,
                         fb_word *out)
{
    if ((out == NULL) || !width_is_supported(width))
    {
        return FB_INVALID;
    }
    if (!field_fits(width, edit.low, edit.high))
    {
        return FB_RANGE;
    }
    fb_word field = mask(edit.high - edit.low);
    if (edit.value > field)
    {
        return FB_RANGE;
    }
    fb_word cleared = input & ~(field << edit.low);
    *out = (cleared | (edit.value << edit.low)) & mask(width);
    return FB_OK;
}

/* Two edits conflict when some bit is written by both with different values.
 * Both edits were already validated by fb_insert. */
static bool edits_conflict(struct fb_edit first, struct fb_edit second)
{
    fb_word first_field = mask(first.high - first.low) << first.low;
    fb_word second_field = mask(second.high - second.low) << second.low;
    fb_word overlap = first_field & second_field;
    fb_word first_bits = first.value << first.low;
    fb_word second_bits = second.value << second.low;
    return ((first_bits ^ second_bits) & overlap) != 0u;
}

enum fb_status fb_patch(unsigned width, fb_word input, struct fb_edit first,
                        struct fb_edit second, enum fb_patch policy,
                        fb_word *out)
{
    if ((out == NULL) || ((unsigned)policy > (unsigned)FB_SIMULTANEOUS))
    {
        return FB_INVALID;
    }

    /* Apply both edits to private copies; *out is written only at the end. */
    fb_word after_first = 0;
    fb_word after_second = 0;
    enum fb_status status = fb_insert(width, input, first, &after_first);
    if (status != FB_OK)
    {
        return status;
    }
    status = fb_insert(width, after_first, second, &after_second);
    if (status != FB_OK)
    {
        return status;
    }
    /* Mutant 2 never reports a simultaneous conflict. */
    if ((policy == FB_SIMULTANEOUS) && (FB_MUTANT != 2) &&
        edits_conflict(first, second))
    {
        return FB_CONFLICT;
    }
    *out = after_second;
    return FB_OK;
}

enum fb_status fb_stats(unsigned width, fb_word input, struct fb_stats *out)
{
    if ((out == NULL) || !width_is_supported(width))
    {
        return FB_INVALID;
    }
    fb_word value = input & mask(width);
    struct fb_stats stats = {0, 0, width};
    for (unsigned bit = 0; bit < width; ++bit)
    {
        if (((value >> bit) & 1u) != 0u)
        {
            ++stats.population;
            stats.width = bit + 1u; /* index of the highest set bit, plus one */
        }
    }
    stats.leading = width - stats.width;
    if ((FB_MUTANT == 3) && (value == 0u))
    {
        stats.leading = 0; /* mutant: wrong policy for the all-zero word */
    }
    *out = stats;
    return FB_OK;
}

enum fb_status fb_encode(fb_word value, enum fb_order order, unsigned char *out,
                         size_t capacity)
{
    if ((out == NULL) || ((unsigned)order > (unsigned)FB_LITTLE))
    {
        return FB_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FB_UNSUPPORTED;
    }
    if (capacity < WIRE_WORD_OCTETS)
    {
        return FB_SHORT;
    }

    unsigned int cells[WIRE_WORD_OCTETS];
    if (fi_encode_cells(value, cells) != FI_OK)
    {
        return FB_RANGE;
    }
    for (unsigned i = 0; i < WIRE_WORD_OCTETS; ++i)
    {
        unsigned source = (order == FB_BIG) ? i : (WIRE_WORD_OCTETS - 1u - i);
        out[i] = (unsigned char)cells[source]; /* cell <= 255: exact */
    }
    return FB_OK;
}

/* A span is usable when it has data or is empty (NULL with size 0). */
static bool span_is_valid(struct fb_span span)
{
    return (span.data != NULL) || (span.size == 0u);
}

enum fb_status fb_decode_two(struct fb_span first, struct fb_span second,
                             enum fb_order order, fb_word *out)
{
    if ((out == NULL) || !span_is_valid(first) || !span_is_valid(second) ||
        ((unsigned)order > (unsigned)FB_LITTLE))
    {
        return FB_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FB_UNSUPPORTED;
    }
    if (FB_MUTANT == 4)
    {
        *out = 0; /* mutant: writes an output even on FB_SHORT */
    }
    /* first.size < 4 and second.size < 4 - first.size are tested instead of
     * first.size + second.size < 4 so the sum cannot wrap. */
    if ((first.size < WIRE_WORD_OCTETS) &&
        (second.size < (WIRE_WORD_OCTETS - first.size)))
    {
        return FB_SHORT;
    }

    /* Gather four octets across the two spans, reading only indices that were
     * just shown to exist, and store them in big-endian cell order. */
    unsigned int cells[WIRE_WORD_OCTETS];
    for (unsigned i = 0; i < WIRE_WORD_OCTETS; ++i)
    {
        unsigned char octet =
            (i < first.size) ? first.data[i] : second.data[i - first.size];
        unsigned destination =
            (order == FB_BIG) ? i : (WIRE_WORD_OCTETS - 1u - i);
        cells[destination] = octet;
    }

    fb_word value = 0;
    if (fi_decode_cells(cells, &value) != FI_OK)
    {
        return FB_RANGE;
    }
    *out = value;
    return FB_OK;
}

enum fb_status fb_decode(struct fb_span input, enum fb_order order,
                         fb_word *out)
{
    const struct fb_span empty = {NULL, 0};
    return fb_decode_two(input, empty, order, out);
}

/* Big-endian 16-bit field. Precondition: two octets are readable at p. */
static unsigned read_u16_be(const unsigned char *p)
{
    return ((unsigned)p[0] * OCTET_RADIX) + p[1];
}

/* Shared header check for records and descriptors: the declared total size
 * must cover at least the header (else FB_RANGE) and must not exceed the
 * octets actually available (else FB_SHORT). Size is never trusted to read
 * beyond input.size. */
static enum fb_status check_declared_size(struct fb_span input, unsigned size)
{
    if (size < HEADER_OCTETS)
    {
        return FB_RANGE;
    }
    if (size > input.size)
    {
        return FB_SHORT;
    }
    return FB_OK;
}

enum fb_status fb_record(struct fb_span input, enum fb_unknown policy,
                         struct fb_record *out)
{
    if ((out == NULL) || !span_is_valid(input) ||
        ((unsigned)policy > (unsigned)FB_SKIP_UNKNOWN))
    {
        return FB_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FB_UNSUPPORTED;
    }
    if (input.size < HEADER_OCTETS)
    {
        return FB_SHORT;
    }

    unsigned version = input.data[0];
    unsigned flags = input.data[1];
    unsigned size = read_u16_be(input.data + HEADER_SIZE_OFFSET);
    enum fb_status status = check_declared_size(input, size);
    if (status != FB_OK)
    {
        return status;
    }

    /* An unknown version or flag means the payload layout is unknown, so it
     * is never read: it is rejected, or skipped using only the declared
     * size. */
    if ((version != RECORD_VERSION) || ((flags & ~RECORD_KNOWN_FLAGS) != 0u))
    {
        if (policy == FB_REJECT_UNKNOWN)
        {
            return FB_UNSUPPORTED;
        }
        *out = (struct fb_record){0, 0, size};
        return FB_SKIPPED;
    }
    if (size != RECORD_TOTAL_OCTETS)
    {
        return FB_RANGE;
    }

    fb_word value = 0;
    const struct fb_span payload = {input.data + HEADER_OCTETS,
                                    WIRE_WORD_OCTETS};
    status = fb_decode(payload, FB_BIG, &value);
    if (status != FB_OK)
    {
        return status;
    }
    *out = (struct fb_record){value, flags, size};
    return FB_OK;
}

/* Minimum descriptor size for a supported version; 0 for unsupported ones. */
static unsigned plugin_minimum_octets(unsigned version)
{
    if (version == 1u)
    {
        return PLUGIN_V1_OCTETS;
    }
    if (version == 2u)
    {
        return PLUGIN_V2_OCTETS;
    }
    return 0u;
}

/* Decode a big-endian word at a fixed offset known to lie inside the declared
 * (and therefore available) descriptor size. */
static enum fb_status plugin_word(struct fb_span input, unsigned offset,
                                  fb_word *out)
{
    const struct fb_span field = {input.data + offset, WIRE_WORD_OCTETS};
    return fb_decode(field, FB_BIG, out);
}

enum fb_status fb_negotiate(struct fb_span input, struct fb_plugin *out)
{
    if ((out == NULL) || !span_is_valid(input))
    {
        return FB_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FB_UNSUPPORTED;
    }
    if (input.size < HEADER_OCTETS)
    {
        return FB_SHORT;
    }

    unsigned version = read_u16_be(input.data);
    unsigned size = read_u16_be(input.data + HEADER_SIZE_OFFSET);
    enum fb_status status = check_declared_size(input, size);
    if (status != FB_OK)
    {
        return status;
    }
    unsigned minimum = plugin_minimum_octets(version);
    if (minimum == 0u)
    {
        return FB_UNSUPPORTED;
    }
    /* The declared size must cover every field this version defines, so no
     * field read below goes past the available prefix. */
    if (size < minimum)
    {
        return FB_SHORT;
    }

    fb_word features = 0;
    fb_word items = 0;
    status = plugin_word(input, PLUGIN_FEATURES_OFFSET, &features);
    if (status != FB_OK)
    {
        return status;
    }
    if ((features & ~(fb_word)PLUGIN_KNOWN_FEATURES) != 0u)
    {
        return FB_UNSUPPORTED;
    }
    if (version == 2u)
    {
        status = plugin_word(input, PLUGIN_MAX_ITEMS_OFFSET, &items);
        if (status != FB_OK)
        {
            return status;
        }
    }
    if ((FB_MUTANT == 6) && (version == 1u))
    {
        items = 999; /* mutant: invents an item limit for version 1 */
    }
    *out = (struct fb_plugin){version, size, features, items};
    return FB_OK;
}

enum fb_status fb_register_init(struct fb_register *reg, unsigned value,
                                unsigned read_clear)
{
    if ((reg == NULL) || (value > REGISTER_VALUE_MAX) || (read_clear > 1u))
    {
        return FB_INVALID;
    }
    *reg = (struct fb_register){(unsigned char)value, 0, 0, read_clear};
    return FB_OK;
}

unsigned fb_register_read(struct fb_register *reg)
{
    unsigned value = reg->state;
    ++reg->reads;
    if (reg->read_clear != 0u)
    {
        reg->state &= REGISTER_ORDINARY_MASK; /* read-to-clear the status */
    }
    return value;
}

void fb_register_write(struct fb_register *reg, unsigned value)
{
    ++reg->writes;
    /* Ordinary bits take the written value; a status bit stays set unless the
     * write has a 1 in that position (write-1-to-clear). */
    unsigned ordinary = value & REGISTER_ORDINARY_MASK;
    unsigned status_kept =
        (reg->state & REGISTER_STATUS_MASK) & ~(value & REGISTER_STATUS_MASK);
    reg->state = (unsigned char)(ordinary | status_kept); /* both <= 0xff */
}

enum fb_status fb_register_update(struct fb_register *reg, unsigned low_value,
                                  unsigned clear_status)
{
    /* low_value is the ordinary nibble; clear_status may only name status
     * bits, so it must not touch the ordinary nibble. */
    if ((reg == NULL) || (low_value > REGISTER_ORDINARY_MASK) ||
        (clear_status > REGISTER_STATUS_MASK) ||
        ((clear_status & REGISTER_ORDINARY_MASK) != 0u))
    {
        return FB_INVALID;
    }
    if (FB_MUTANT == 5)
    {
        /* Mutant: a read-modify-write that performs an unrequested read. */
        unsigned value = fb_register_read(reg);
        fb_register_write(reg, (value & REGISTER_STATUS_MASK) | low_value |
                                   clear_status);
    }
    else
    {
        fb_register_write(reg, low_value | clear_status);
    }
    return FB_OK;
}
