#include "errors.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    char op;
    uint32_t a, b, policy;
    while (scanf(" %c %" SCNu32 " %" SCNu32 " %" SCNu32, &op, &a, &b,
                 &policy) == 4)
    {
        if (op == 'A')
        {
            uint32_t output = 77;
            er_code code = er_add16(a, b, (er_policy)policy, &output);
            printf("%d %" PRIu32 "\n", (int)code, output);
        }
        else if (op == 'F')
        {
            er_fraction output = {91, 92};
            er_code code = er_refine(a, b, &output);
            printf("%d %" PRIu32 " %" PRIu32 "\n", (int)code, output.numerator,
                   output.denominator);
        }
        else
            return 2;
    }
    return ferror(stdin) ? 2 : 0;
}
