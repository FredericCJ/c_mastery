#include "execution.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    unsigned mode, op, expected, count = 0;
    int32_t a, b, want;
    while (scanf("%u %u %" SCNd32 " %" SCNd32 " %u %" SCNd32, &mode, &op, &a,
                 &b, &expected, &want) == 6)
    {
        int32_t got = 91;
        int status = vs_arithmetic(mode, op, a, b, &got);
        if (status != (int)expected || got != want)
        {
            printf("VS_ORACLE_FAIL case=%u status=%d expected=%u\n", count,
                   status, expected);
            return 1;
        }
        if (++count > 18696)
            return 2;
    }
    if (!feof(stdin) || count != 18696)
        return 2;
    printf("VS_ORACLE_PASS cases=%u integer_modular_models\n", count);
    return 0;
}
