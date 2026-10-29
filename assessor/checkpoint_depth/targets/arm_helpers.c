/* Explicit AAPCS byte-clear provider observed in Clang's aggregate initialization. */
#include <stddef.h>

void __aeabi_memclr4(void *destination, size_t n)
{
    volatile unsigned char *out = destination;
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = 0;
    }
}
