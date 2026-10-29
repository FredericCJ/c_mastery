#include "alias.h"
#include <stdio.h>

int main(void)
{
    for (size_t length = 0; length <= 8; ++length)
    {
        for (size_t dst = 0; dst <= length + 1; ++dst)
        {
            for (size_t src = 0; src <= length + 1; ++src)
            {
                for (size_t n = 0; n <= length + 1; ++n)
                {
                    for (int inplace = 0; inplace <= 1; ++inplace)
                    {
                        unsigned char a[8];
                        for (size_t i = 0; i < 8; ++i)
                        {
                            a[i] = (unsigned char)i;
                        }
                        int result =
                            os_partition(a, length, dst, src, n, 0x55, inplace);
                        printf("%zu,%zu,%zu,%zu,%d,%d", length, dst, src, n,
                               inplace, result);
                        for (size_t i = 0; i < 8; ++i)
                        {
                            printf(",%u", a[i]);
                        }
                        putchar('\n');
                    }
                }
            }
        }
    }
    return 0;
}
