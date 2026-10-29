/* Declared target-runtime providers, authored from byte operations. No libc link. */
#include <stddef.h>

void *memcpy(void *restrict destination, const void *restrict source, size_t n)
{
    volatile unsigned char *out = destination;
    const volatile unsigned char *in = source;
    for (size_t i = 0; i < n; ++i)
        out[i] = in[i];
    return destination;
}

void *memset(void *destination, int value, size_t n)
{
    volatile unsigned char *out = destination;
    for (size_t i = 0; i < n; ++i)
        out[i] = (unsigned char)value;
    return destination;
}
