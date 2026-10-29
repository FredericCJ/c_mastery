#include "money.h"
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

int main(void)
{
    for (unsigned n = 1; n <= 32; n *= 2)
    {
        uint32_t w[32];
        struct cm_money out[32], m = {1000003, {'U', 'S', 'D'}, 2};
        int64_t checksum = 0;
        for (unsigned i = 0; i < n; i++)
            w[i] = i + 1;
        clock_t start = clock();
        for (unsigned i = 0; i < 10000; i++)
        {
            if (cm_allocate(m, w, n, out) != CM_OK)
                return 2;
            checksum += out[i % n].minor;
        }
        printf("recipients=%u iterations=10000 ticks=%" PRIdMAX
               " checksum=%" PRId64 " money_bytes=%zu\n",
               n, (intmax_t)(clock() - start), checksum,
               sizeof(struct cm_money));
    }
    return 0;
}
