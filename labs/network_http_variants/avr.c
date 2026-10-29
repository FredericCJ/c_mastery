#include "variant.h"
#include "http_depth.h"
#include <limits.h>
_Static_assert(sizeof(size_t) == 2 && SIZE_MAX == 65535u,
               "Actual16bit extent required");
_Static_assert(sizeof(uint32_t) == 4, "Actual32bit wire value required");

static void put(unsigned char c)
{
    while (!(*(volatile unsigned char *)0xc0 & 0x20))
    {
    }
    *(volatile unsigned char *)0xc6 = c;
}

int main(void)
{
    *(volatile unsigned char *)0xc1 = 0x08;
    volatile unsigned char seed = 0;
    unsigned char s[10];
    uint32_t out = 77;
    int pass = 1;
    s[0] = (unsigned char)('9' + seed);
    pass &= nv_length(s, 1, 0, 1024, &out) == 400 && out == 77;
    s[0] = '0';
    pass &= nv_length(s, 1, 0, 0, &out) == 0 && out == 0;
    s[0] = '1';
    s[1] = '0';
    s[2] = '2';
    s[3] = '5';
    out = 77;
    pass &= nv_length(s, 4, UINT32_MAX, 1024, &out) == 413 && out == 77;
    s[4] = 'x';
    pass &= nv_length(s, 5, UINT32_MAX, 1024, &out) == 400 && out == 77;
    s[0] = '4';
    s[1] = '2';
    s[2] = '9';
    s[3] = '4';
    s[4] = '9';
    s[5] = '6';
    s[6] = '7';
    s[7] = '2';
    s[8] = '9';
    s[9] = '5';
    pass &= nv_length(s, 10, UINT32_MAX, 1024, &out) == 413 &&
            hd_length(s, 10, 1024, &out) == 413 && out == 77;
    s[9] = '6';
    pass &= nv_length(s, 10, UINT32_MAX, 1024, &out) == 400 &&
            hd_length(s, 10, 1024, &out) == 400 && out == 77;
    s[0] = '1';
    s[1] = '0';
    s[2] = '2';
    s[3] = '4';
    pass &= nv_length(s, 4, 1024, 1024, &out) == 0 && out == 1024;
    size_t extent = (size_t)out;
    pass &= extent == 1024;
    put('A');
    put('V');
    put('R');
    put(':');
    put(pass ? 'P' : 'F');
    put('A');
    put(pass ? 'S' : 'I');
    put(pass ? 'S' : 'L');
    put('\n');
    for (;;)
        __asm__ volatile("sleep");
}
