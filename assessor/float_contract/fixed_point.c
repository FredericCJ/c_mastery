/*
 * Exercise of the earlier fixed-point multiply (bf_mul16) imported unchanged
 * from the b02 follow-through provider, with ties-to-even rounding (BF_EVEN)
 * and an int8 output. Prints the raw rounded product and saturated output for
 * six pairs so the campaign can compare them with an independent rational
 * computation.
 */
#include "fixed_point.h"
#include <stdio.h>

int main(void)
{
    /* Extremes of int8, small fractional products and mixed signs. */
    const int pairs[][2] = {{-128, -128}, {-1, 8},  {1, 8},
                            {127, 127},   {16, 16}, {3, -8}};
    const size_t pair_count = sizeof pairs / sizeof pairs[0];

    for (size_t i = 0; i < pair_count; i++)
    {
        struct bf_small result;
        /* Operands are within int8 range by construction. */
        int status = bf_mul16((int8_t)pairs[i][0], (int8_t)pairs[i][1], BF_EVEN,
                              &result);
        if (status != B2_OK)
        {
            return 1;
        }
        printf("FIXED %d %d %ld %d\n", pairs[i][0], pairs[i][1],
               (long)result.rounded_raw, (int)result.output);
    }
    puts("FF_FIXED_PASS prior_provider scale16 ties_even saturation");
    return 0;
}
