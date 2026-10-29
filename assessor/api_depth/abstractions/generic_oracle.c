#include "generic.h"
#include <stdio.h>

int main(void)
{
    size_t n;
    unsigned byte;
    unsigned char bytes[8];
    while (scanf("%zu", &n) == 1)
    {
        if (n > 8)
            return 2;
        for (size_t i = 0; i < n; ++i)
        {
            if (scanf("%u", &byte) != 1 || byte > 255)
                return 3;
            bytes[i] = (unsigned char)byte;
        }
        struct as_value v = av_parse(bytes, n);
        if (v.tag == AV_INT)
            printf("I %d\n", v.as.integer);
        else if (v.tag == AV_ERROR)
            printf("E %d %zu\n", v.as.error.code, v.as.error.offset);
        else
            return 4;
    }
    return 0;
}
