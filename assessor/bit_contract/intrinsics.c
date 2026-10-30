/*
 * Comparison of fb_stats with the compiler intrinsics __builtin_popcount and
 * __builtin_clz over every 16-bit value at every width 1..16.
 *
 * The intrinsics are a separate, capability-checked mechanism: clz is
 * undefined for zero, so the wrapper below guards zero itself. Defining
 * FB_INTRINSIC_ZERO selects a deliberately wrong zero policy, which this
 * program must detect (the campaign expects a nonzero exit).
 */
#include "bits.h"
#include <limits.h>
#include <stdio.h>
#if !defined(__has_builtin)
#error builtin_capability_missing
#endif
#if !__has_builtin(__builtin_popcount) || !__has_builtin(__builtin_clz)
#error bit_builtin_capability_missing
#endif
_Static_assert(UINT_MAX == 4294967295u,
               "recorded32bit unsigned int intrinsic carrier");

#define MAX_LOGICAL_WIDTH_BITS 16u
#define UNSIGNED_INT_BITS 32u /* matches the _Static_assert above */
#define LOGICAL_VALUE_COUNT 65536u

/* fb_stats expressed with the intrinsics. width is the logical field width in
 * bits (1..16); input bits above it are discarded first. */
static enum fb_status intrinsic(unsigned width, fb_word input,
                                struct fb_stats *out)
{
    if ((out == NULL) || (width == 0u) || (width > MAX_LOGICAL_WIDTH_BITS))
    {
        return FB_INVALID;
    }

    unsigned value = (unsigned)(input & (((fb_word)1 << width) - 1u));
    /* __builtin_clz counts leading zeros of the whole unsigned int, so the
     * zeros above the logical field are subtracted. */
    unsigned leading = width;
    if (value != 0u)
    {
        unsigned zeros_in_word = (unsigned)__builtin_clz(value);
        leading = zeros_in_word - (UNSIGNED_INT_BITS - width);
    }
#ifdef FB_INTRINSIC_ZERO
    if (value == 0u)
    {
        leading = 0; /* deliberate wrong zero policy; never calls clz(0) */
    }
#endif
    out->population = (unsigned)__builtin_popcount(value);
    out->width = width - leading;
    out->leading = leading;
    return FB_OK;
}

int main(void)
{
    unsigned long cases = 0;
    for (unsigned width = 1; width <= MAX_LOGICAL_WIDTH_BITS; ++width)
    {
        for (unsigned n = 0; n < LOGICAL_VALUE_COUNT; ++n)
        {
            struct fb_stats reference = {0, 0, 0};
            struct fb_stats candidate = {0, 0, 0};
            if (fb_stats(width, n, &reference) != FB_OK ||
                intrinsic(width, n, &candidate) != FB_OK ||
                reference.population != candidate.population ||
                reference.width != candidate.width ||
                reference.leading != candidate.leading)
            {
                fprintf(stderr, "FB_INTRINSIC_FAIL width=%u value=%u\n", width,
                        n);
                return 1;
            }
            ++cases;
        }
    }
    printf("FB_INTRINSIC_PASS cases=%lu zero_guard logical_width\n", cases);
    return 0;
}
