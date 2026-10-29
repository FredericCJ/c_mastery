#include <stddef.h>
#include <stdint.h>

void *memcpy(void *out, const void *in, size_t n)
{
    unsigned char *d = out;
    const unsigned char *s = in;
    while (n--)
    {
        *d++ = *s++;
    }
    return out;
}

void *memset(void *out, int value, size_t n)
{
    unsigned char *d = out;
    while (n--)
    {
        *d++ = (unsigned char)value;
    }
    return out;
}

/* Flat-address x86-64 guest adapter; unrelated portable C pointers need not
 * possess this integer-address ordering. */
void *memmove(void *out, const void *in, size_t n)
{
    unsigned char *d = out;
    const unsigned char *s = in;
    if ((uintptr_t)d < (uintptr_t)s)
    {
        while (n--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        while (n)
        {
            --n;
            d[n] = s[n];
        }
    }
    return out;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    while (n--)
    {
        if (*x != *y)
        {
            return *x < *y ? -1 : 1;
        }
        ++x;
        ++y;
    }
    return 0;
}
