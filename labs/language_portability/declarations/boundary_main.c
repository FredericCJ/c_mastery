#include "boundary.h"
#include "declarations.h"
#include <stdio.h>

int main(void)
{
    for (int32_t v = -4097; v <= 4096; v++)
    {
        unsigned char a[2] = {165, 90}, b[2] = {165, 90};
        int x = ld_bit_encode(v, a), y = ld_encode(v, b) == LD_OK;
        if (x != y || a[0] != b[0] || a[1] != b[1])
        {
            return 1;
        }
    }
    for (unsigned int w = 0; w < 65536; w++)
    {
        unsigned char a[2] = {(unsigned char)(w >> 8), (unsigned char)w};
        int32_t x = 99, y = 99;
        int r = ld_bit_decode(a, &x), s = ld_decode(a, &y) == LD_OK;
        if (r != s || x != y)
        {
            return 1;
        }
    }
    puts(
        "LD_BOUNDARY 8194 values 65536 encodings equivalent explicit-standard-type-boundary");
    return 0;
}
