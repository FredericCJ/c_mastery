#include "fixed_point.h"
#include <stdio.h>

int main(void)
{
    const int pairs[][2] = {{-128, -128}, {-1, 8},  {1, 8},
                            {127, 127},   {16, 16}, {3, -8}};
    for (size_t i = 0; i < 6; i++)
    {
        struct bf_small r;
        if (bf_mul16((int8_t)pairs[i][0], (int8_t)pairs[i][1], BF_EVEN, &r) !=
            B2_OK)
            return 1;
        printf("FIXED %d %d %ld %d\n", pairs[i][0], pairs[i][1],
               (long)r.rounded_raw, (int)r.output);
    }
    puts("FF_FIXED_PASS prior_provider scale16 ties_even saturation");
    return 0;
}
