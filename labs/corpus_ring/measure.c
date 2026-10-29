#include "ring.h"
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

int main(void)
{
    for (unsigned layout = 0; layout < 4; layout++)
        for (unsigned n = 4; n <= 32; n *= 2)
        {
            uint32_t state = 760010;
            struct cr_token t[32];
            for (unsigned i = 0; i < n; i++)
            {
                state = state * 1664525u + 1013904223u;
                uint16_t p = layout == 0   ? (uint16_t)(65536u / n * i)
                             : layout == 1 ? (uint16_t)i
                             : layout == 2 ? 0
                                           : (uint16_t)(state >> 16);
                t[i] = (struct cr_token){p, (uint8_t)(i % 16 + 1),
                                         (uint8_t)(i / 16)};
            }
            struct cr_ring r;
            if (cr_build(t, n, 1, &r) != CR_OK)
                return 2;
            uint64_t compares = 0, checksum = 0;
            unsigned loads[16] = {0};
            clock_t start = clock();
            for (unsigned key = 0; key < 65536; key++)
            {
                unsigned owner;
                struct cr_cost c;
                if (cr_owner(&r, (uint16_t)key, 1, &owner, &c) != CR_OK)
                    return 3;
                compares += c.comparisons;
                checksum += owner;
                loads[owner - 1]++;
            }
            unsigned min = 65536, max = 0;
            for (unsigned i = 0; i < (n < 16 ? n : 16); i++)
            {
                if (loads[i] < min)
                    min = loads[i];
                if (loads[i] > max)
                    max = loads[i];
            }
            printf("layout=%u tokens=%u keys=65536 comparisons=%" PRIu64
                   " checksum=%" PRIu64
                   " load_min=%u load_max=%u ticks=%" PRIdMAX
                   " ring_bytes=%zu\n",
                   layout, n, compares, checksum, min, max,
                   (intmax_t)(clock() - start), sizeof r);
        }
    return 0;
}
