/*
 * Independent oracle for the bit operations in bits.h.
 *
 * The oracle stores a value as a list of bits (index 0 is the least
 * significant) and rebuilds integers by summing powers of two. It does not
 * repeat the reference's mask-and-shift formulae: shifts, rotations, fields
 * and edits are expressed as moves of list elements.
 */
#include "bits.h"
#include <stdbool.h>
#include <stdio.h>

/* width and input must be in scope at the point of use. */
#define T(x)                                                              \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "FB_ORACLE_FAIL line=%d width=%u input=%u\n", \
                    __LINE__, width, input);                              \
            return 1;                                                     \
        }                                                                 \
    } while (0)

/* Sentinel proving an output was left untouched on failure. */
#define UNTOUCHED 917

/* Exhaustive bounds. Values and widths up to 8 bits are enumerated; the
 * two-edit patch space is enumerated over a 4-bit word. */
#define BIT_LIST_CAPACITY 8u
#define MAX_ORACLE_WIDTH 8u
#define PATCH_WIDTH 4u
#define SHIFT_POLICIES 3u

struct oracle_counts
{
    unsigned long stats;
    unsigned long shifts;
    unsigned long fields;
    unsigned long patches;
};

static unsigned power_of_two(unsigned n)
{
    unsigned p = 1;
    while (n-- > 0u)
    {
        p *= 2u;
    }
    return p;
}

/* Sum of bits[i] * 2^i for i < width. */
static unsigned bits_to_value(const unsigned bits[BIT_LIST_CAPACITY],
                              unsigned width)
{
    unsigned value = 0;
    for (unsigned i = 0; i < width; ++i)
    {
        value += bits[i] * power_of_two(i);
    }
    return value;
}

/* Repeated division by two, least significant bit first. */
static void value_to_bits(unsigned value, unsigned width,
                          unsigned bits[BIT_LIST_CAPACITY])
{
    for (unsigned i = 0; i < width; ++i)
    {
        bits[i] = value % 2u;
        value /= 2u;
    }
}

static int check_stats(unsigned width, unsigned input,
                       const unsigned bits[BIT_LIST_CAPACITY],
                       struct oracle_counts *counts)
{
    unsigned population = 0;
    unsigned highest_plus_one = 0;
    for (unsigned i = 0; i < width; ++i)
    {
        if (bits[i] != 0u)
        {
            ++population;
            highest_plus_one = i + 1u;
        }
    }

    struct fb_stats actual = {0, 0, 0};
    T(fb_stats(width, input, &actual) == FB_OK &&
      actual.population == population && actual.width == highest_plus_one &&
      actual.leading == width - highest_plus_one);
    ++counts->stats;
    return 0;
}

/* Expected bit list after a shift or rotation by count (0 <= count < width),
 * as element moves: position i takes the element `count` places away. */
static void expected_shift(unsigned policy, unsigned count, unsigned width,
                           const unsigned bits[BIT_LIST_CAPACITY],
                           unsigned want[BIT_LIST_CAPACITY])
{
    for (unsigned i = 0; i < width; ++i)
    {
        if (policy == (unsigned)FB_LEFT)
        {
            want[i] = (i >= count) ? bits[i - count] : 0u;
        }
        else if (policy == (unsigned)FB_RIGHT)
        {
            want[i] = ((i + count) < width) ? bits[i + count] : 0u;
        }
        else
        {
            want[i] = bits[(i + width - count) % width];
        }
    }
}

/* Counts from -1 to width inclusive: the first and last are out of range and
 * must be rejected without touching the output. */
static int check_shifts(unsigned width, unsigned input,
                        const unsigned bits[BIT_LIST_CAPACITY],
                        struct oracle_counts *counts)
{
    for (int count = -1; count <= (int)width; ++count)
    {
        for (unsigned policy = 0; policy < SHIFT_POLICIES; ++policy)
        {
            fb_word out = UNTOUCHED;
            if ((count < 0) || (count == (int)width))
            {
                T(fb_shift(width, input, count, (enum fb_shift)policy, &out) ==
                      FB_RANGE &&
                  out == UNTOUCHED);
            }
            else
            {
                unsigned want[BIT_LIST_CAPACITY] = {0};
                expected_shift(policy, (unsigned)count, width, bits, want);
                T(fb_shift(width, input, count, (enum fb_shift)policy, &out) ==
                      FB_OK &&
                  out == bits_to_value(want, width));
            }
            ++counts->shifts;
        }
    }
    return 0;
}

/* Extraction of every half-open field [lo, hi), and insertion of every
 * representable field value plus the first value that does not fit. */
static int check_fields(unsigned width, unsigned input,
                        const unsigned bits[BIT_LIST_CAPACITY],
                        struct oracle_counts *counts)
{
    for (unsigned lo = 0; lo <= width; ++lo)
    {
        for (unsigned hi = lo; hi <= width; ++hi)
        {
            fb_word out = UNTOUCHED;
            unsigned want[BIT_LIST_CAPACITY] = {0};
            for (unsigned i = lo; i < hi; ++i)
            {
                want[i - lo] = bits[i];
            }
            T(fb_extract(width, input, lo, hi, &out) == FB_OK &&
              out == bits_to_value(want, hi - lo));

            unsigned field_values = power_of_two(hi - lo);
            for (unsigned v = 0; v <= field_values; ++v)
            {
                out = UNTOUCHED;
                const struct fb_edit edit = {lo, hi, v};
                if (v == field_values)
                {
                    T(fb_insert(width, input, edit, &out) == FB_RANGE &&
                      out == UNTOUCHED);
                }
                else
                {
                    unsigned expected[BIT_LIST_CAPACITY] = {0};
                    for (unsigned i = 0; i < width; ++i)
                    {
                        expected[i] = bits[i];
                    }
                    unsigned remaining = v;
                    for (unsigned i = lo; i < hi; ++i)
                    {
                        expected[i] = remaining % 2u;
                        remaining /= 2u;
                    }
                    T(fb_insert(width, input, edit, &out) == FB_OK &&
                      out == bits_to_value(expected, width));
                }
                ++counts->fields;
            }
        }
    }
    return 0;
}

/* Two independently specified edits of a PATCH_WIDTH-bit word. Sequential
 * order is last-writer-wins. Simultaneous editing conflicts exactly when some
 * bit is written by both edits with different values. */
static int check_patch(unsigned input, struct fb_edit first,
                       struct fb_edit second, struct oracle_counts *counts)
{
    const unsigned width = PATCH_WIDTH;
    unsigned bits[BIT_LIST_CAPACITY] = {0};
    unsigned first_written[BIT_LIST_CAPACITY] = {0};
    unsigned first_bit[BIT_LIST_CAPACITY] = {0};
    bool conflict = false;

    value_to_bits(input, width, bits);
    unsigned remaining_first = (unsigned)first.value;
    for (unsigned i = first.low; i < first.high; ++i)
    {
        bits[i] = remaining_first % 2u;
        remaining_first /= 2u;
        first_written[i] = 1u;
        first_bit[i] = bits[i];
    }
    unsigned remaining_second = (unsigned)second.value;
    for (unsigned i = second.low; i < second.high; ++i)
    {
        unsigned bit = remaining_second % 2u;
        remaining_second /= 2u;
        if ((first_written[i] != 0u) && (first_bit[i] != bit))
        {
            conflict = true;
        }
        bits[i] = bit;
    }

    fb_word out = UNTOUCHED;
    T(fb_patch(width, input, first, second, FB_SEQUENTIAL, &out) == FB_OK &&
      out == bits_to_value(bits, width));
    out = UNTOUCHED;
    T(fb_patch(width, input, first, second, FB_SIMULTANEOUS, &out) ==
          (conflict ? FB_CONFLICT : FB_OK) &&
      out == (conflict ? UNTOUCHED : bits_to_value(bits, width)));
    ++counts->patches;
    return 0;
}

static int check_all_patches(struct oracle_counts *counts)
{
    for (unsigned input = 0; input < power_of_two(PATCH_WIDTH); ++input)
    {
        for (unsigned lo = 0; lo <= PATCH_WIDTH; ++lo)
        {
            for (unsigned hi = lo; hi <= PATCH_WIDTH; ++hi)
            {
                for (unsigned lo2 = 0; lo2 <= PATCH_WIDTH; ++lo2)
                {
                    for (unsigned hi2 = lo2; hi2 <= PATCH_WIDTH; ++hi2)
                    {
                        for (unsigned av = 0; av < power_of_two(hi - lo); ++av)
                        {
                            for (unsigned bv = 0; bv < power_of_two(hi2 - lo2);
                                 ++bv)
                            {
                                const struct fb_edit first = {lo, hi, av};
                                const struct fb_edit second = {lo2, hi2, bv};
                                if (check_patch(input, first, second, counts) !=
                                    0)
                                {
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    struct oracle_counts counts = {0, 0, 0, 0};

    for (unsigned width = 1; width <= MAX_ORACLE_WIDTH; ++width)
    {
        for (unsigned input = 0; input < power_of_two(MAX_ORACLE_WIDTH);
             ++input)
        {
            unsigned bits[BIT_LIST_CAPACITY] = {0};
            value_to_bits(input, width, bits);
            if (check_stats(width, input, bits, &counts) ||
                check_shifts(width, input, bits, &counts) ||
                check_fields(width, input, bits, &counts))
            {
                return 1;
            }
        }
    }
    if (check_all_patches(&counts) != 0)
    {
        return 1;
    }
    printf("FB_ORACLE_PASS stats=%lu shifts=%lu fields=%lu patches=%lu\n",
           counts.stats, counts.shifts, counts.fields, counts.patches);
    return 0;
}
