#include "arch.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    int64_t x;
    while (scanf("%" SCNd64, &x) == 1)
    {
        if (x < INT32_MIN || x > INT32_MAX)
            return 2;
        int32_t input = (int32_t)x, out = 77;
        int status = ar_double(&input, 1, &out);
        printf("%d %" PRId32 "\n", status, out);
    }
    return 0;
}
