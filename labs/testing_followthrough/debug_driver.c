#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
uint32_t debug_decode(const unsigned char *, size_t, unsigned);
unsigned char debug_bytes[2] = {0x12, 0x34};
volatile uint32_t debug_observed;

int main(int argc, char **argv)
{
    size_t extent = 2;
    if (argc == 3 || argc == 4)
    {
        char *a, *b;
        unsigned long x = strtoul(argv[1], &a, 16),
                      y = strtoul(argv[2], &b, 16);
        if (*a || *b || x > 255 || y > 255)
            return 2;
        debug_bytes[0] = (unsigned char)x;
        debug_bytes[1] = (unsigned char)y;
        if (argc == 4)
        {
            if (argv[3][1] || argv[3][0] < '0' || argv[3][0] > '2')
                return 2;
            extent = (size_t)(argv[3][0] - '0');
        }
    }
    else if (argc != 1)
        return 2;
    debug_observed = debug_decode(debug_bytes, extent, 1);
    if (extent != 2)
    {
        if (debug_observed != UINT32_MAX)
            return 1;
        puts("DEBUG_REJECT:extent");
        return 0;
    }
    uint32_t expected = ((uint32_t)debug_bytes[0] << 8) | debug_bytes[1];
    printf("DEBUG_VALUE:%u:EXPECTED:%u\n", debug_observed, expected);
    if (debug_observed != expected)
    {
        fputs("DEBUG_CHECK:wire-big-endian\n", stderr);
        return 1;
    }
    return 0;
}
