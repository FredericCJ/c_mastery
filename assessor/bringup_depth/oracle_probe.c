#include "bringup.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    for (unsigned bits = 0; bits < 256; bits++)
    {
        struct bu_handoff h = {
            0x20030000,      0x08000200,      0x08000401,      (bits >> 0) & 1,
            (bits >> 1) & 1, (bits >> 2) & 1, (bits >> 3) & 1, (bits >> 4) & 1,
            (bits >> 5) & 1, (bits >> 6) & 1, (bits >> 7) & 1};
        printf("H %u %u\n", bits,
               (unsigned)bu_handoff(&h, (struct bu_range){0x08000000, 0x200000},
                                    (struct bu_range){0x2002f000, 0x1000}));
    }
    for (unsigned at = 0; at < 32; at++)
    {
        for (unsigned len = 0; len <= 8; len++)
        {
            unsigned char ram[32], rom[32];
            memset(ram, 0xa5, sizeof ram);
            for (unsigned i = 0; i < 32; i++)
            {
                rom[i] = (unsigned char)(3 * i + 1);
            }
            struct bu_copy cp[2] = {{0, 2, 3}, {17, at, len}};
            struct bu_range z = {16, 4};
            unsigned result = (unsigned)bu_memory(ram, 32, rom, 32, cp, 2, &z,
                                                  1, (struct bu_range){24, 4});
            printf("M %u %u %u", at, len, result);
            for (unsigned i = 0; i < 32; i++)
            {
                printf(" %u", ram[i]);
            }
            putchar('\n');
        }
    }
    /* Host emits observations; Python independently derives interval membership.
     */
    for (unsigned a = 0; a < 16; a++)
    {
        for (unsigned n = 0; n < 9; n++)
        {
            for (unsigned b = 0; b < 24; b++)
            {
                for (unsigned m = 0; m < 9; m++)
                {
                    printf("R %u %u %u %u %d %d\n", a, n, b, m,
                           bu_contains((struct bu_range){a, n},
                                       (struct bu_range){b, m}),
                           bu_overlap((struct bu_range){a, n},
                                      (struct bu_range){b, m}));
                }
            }
        }
    }
    for (unsigned x = 0; x < 256; x++)
    {
        for (unsigned y = 0; y < 256; y++)
        {
            unsigned char slots[2][BU_RECORD];
            struct bu_record a = {1, 4, (unsigned char)x, {1, 2, 3, 4}},
                             b = {1, 4, (unsigned char)y, {4, 3, 2, 1}};
            unsigned writes, which = 9;
            if (bu_store(slots[0], BU_RECORD, &a, 25, &writes) ||
                bu_store(slots[1], BU_RECORD, &b, 25, &writes))
            {
                return 2;
            }
            enum bu_result r =
                bu_select((const unsigned char (*)[BU_RECORD])slots, &which);
            printf("G %u %u %u %u\n", x, y, (unsigned)r, which);
        }
    }
    return 0;
}
