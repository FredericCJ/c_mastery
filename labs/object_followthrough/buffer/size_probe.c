/*
 * Exhaustive enumeration of ob_size() over boundary values. One CSV line per
 * combination: prefix,width,count,limit,status,out (all in bytes or
 * elements; out stays 99 when the request was rejected).
 */
#include "buffer.h"
#include <stdio.h>

#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

int main(void)
{
    const size_t prefixes[] = {0, 16, sizeof(struct ob_flex), SIZE_MAX};
    const size_t widths[] = {1, 4, 8, SIZE_MAX};
    const size_t counts[] = {0, 1, 3, 100, SIZE_MAX};
    const size_t limits[] = {0, 15, 255, SIZE_MAX};
    for (size_t p = 0; p < COUNT_OF(prefixes); ++p)
    {
        for (size_t w = 0; w < COUNT_OF(widths); ++w)
        {
            for (size_t c = 0; c < COUNT_OF(counts); ++c)
            {
                for (size_t l = 0; l < COUNT_OF(limits); ++l)
                {
                    size_t out = 99;
                    int status = ob_size(prefixes[p], widths[w], counts[c],
                                         limits[l], &out);
                    printf("%zu,%zu,%zu,%zu,%d,%zu\n", prefixes[p], widths[w],
                           counts[c], limits[l], status, out);
                }
            }
        }
    }
    return 0;
}
