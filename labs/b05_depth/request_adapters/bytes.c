#include <stddef.h>

/* Original target byte loops, not a host runtime dependency. */
void *memset(void *p, int value, size_t n)
{
    volatile unsigned char *d = p;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = (unsigned char)value;
    }
    return p;
}

void *memcpy(void *p, const void *q, size_t n)
{
    volatile unsigned char *d = p;
    const volatile unsigned char *s = q;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
    return p;
}
