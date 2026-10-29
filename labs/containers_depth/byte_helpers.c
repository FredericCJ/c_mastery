/* Original compiler ABI helpers for audited freestanding aggregate lowering. */
#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t n)
{
    unsigned char *d = destination;
    const unsigned char *s = source;
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = s[i];
    }
    return destination;
}

void *memset(void *destination, int value, size_t n)
{
    unsigned char *d = destination;
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = (unsigned char)value;
    }
    return destination;
}

/* Aggregate lowering in this family requires no memmove; do not invent a pointer-order contract. */
