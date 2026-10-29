#include "reduction.h"
#include <stdio.h>

int main(void)
{
    uint64_t values[257];
    uint8_t bytes[257];
    atomic_uint visits[257];
    for (unsigned i = 0; i < 257; i++)
    {
        values[i] = (uint64_t)(i + 1) * (i + 1);
        bytes[i] = (uint8_t)((i * 7 + 3) % 16);
    }
    for (unsigned n = 0; n <= 257; n++)
        for (unsigned workers = 1; workers <= 4; workers++)
        {
            struct sr_world w;
            struct sr_result u, b;
            if (sr_init(&w, values, visits, n, workers, SR_UNSIGNED) != SR_OK)
                return 1;
            for (unsigned i = 0; i < workers; i++)
                if (sr_work(&w, i, SIZE_MAX) != SR_OK)
                    return 1;
            if (sr_finish(&w, &u) != SR_OK)
                return 1;
            if (sr_init(&w, bytes, visits, n, workers, SR_BYTES) != SR_OK)
                return 1;
            for (unsigned i = workers; i; i--)
                if (sr_work(&w, i - 1, SIZE_MAX) != SR_OK)
                    return 1;
            if (sr_finish(&w, &b) != SR_OK)
                return 1;
            printf("SR_DIFF n=%u workers=%u sum=%llu hash=%llu count=%zu", n,
                   workers, (unsigned long long)u.unsigned_sum,
                   (unsigned long long)b.hash, b.count);
            for (unsigned i = 0; i < 16; i++)
                printf(" b%u=%llu", i, (unsigned long long)b.bins[i]);
            puts("");
        }
    return 0;
}
