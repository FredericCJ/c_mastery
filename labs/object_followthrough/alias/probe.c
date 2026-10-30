/*
 * Exhaustive enumeration of os_partition() over a small array.
 *
 * One CSV line per case:
 *   length,dst,src,count,inplace,result,b0,b1,...,b7
 * where b0..b7 are the array contents after the call. The array always has
 * ARRAY_BYTES elements holding 0..7, but only the first `length` elements are
 * presented to the function; dst, src and count run past `length` on purpose
 * so that out-of-range requests are part of the enumeration.
 */
#include "alias.h"
#include <stdio.h>

enum
{
    ARRAY_BYTES = 8,
    XOR_MASK = 0x55
};

static void run_case(size_t length, size_t dst, size_t src, size_t count,
                     int inplace)
{
    unsigned char array[ARRAY_BYTES];
    for (size_t i = 0; i < ARRAY_BYTES; ++i)
    {
        array[i] = (unsigned char)i;
    }
    int result =
        os_partition(array, length, dst, src, count, XOR_MASK, inplace);
    printf("%zu,%zu,%zu,%zu,%d,%d", length, dst, src, count, inplace, result);
    for (size_t i = 0; i < ARRAY_BYTES; ++i)
    {
        printf(",%u", array[i]);
    }
    putchar('\n');
}

int main(void)
{
    for (size_t length = 0; length <= ARRAY_BYTES; ++length)
    {
        for (size_t dst = 0; dst <= length + 1; ++dst)
        {
            for (size_t src = 0; src <= length + 1; ++src)
            {
                for (size_t count = 0; count <= length + 1; ++count)
                {
                    for (int inplace = 0; inplace <= 1; ++inplace)
                    {
                        run_case(length, dst, src, count, inplace);
                    }
                }
            }
        }
    }
    return 0;
}
