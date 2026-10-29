/* Original byte-fill support for compiler lowering of the bounded cache init.
 * Volatile byte accesses prevent a recursive lowering into another memset. */
#include <stddef.h>

void *memset(void *destination, int value, size_t n)
{
    volatile unsigned char *d = destination;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = (unsigned char)value;
    }
    return destination;
}
