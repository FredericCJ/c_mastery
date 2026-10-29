#include <stddef.h>

/* These original byte routines close compiler-generated memory calls. No
 * external libc/compiler archive is linked. Their symbols are intentionally
 * visible in the image map, rather than hidden by a no-library claim. */
void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *a = d;
    const unsigned char *b = s;
    for (size_t i = 0; i < n; i++)
    {
        a[i] = b[i];
    }
    return d;
}

void *memset(void *d, int v, size_t n)
{
    unsigned char *a = d;
    for (size_t i = 0; i < n; i++)
    {
        a[i] = (unsigned char)v;
    }
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    unsigned char *a = d;
    const unsigned char *b = s; /* target address-order contract, not generic
                                   unrelated C pointer ordering */
    if ((size_t)a < (size_t)b)
    {
        for (size_t i = 0; i < n; i++)
        {
            a[i] = b[i];
        }
    }
    else
    {
        while (n)
        {
            n--;
            a[n] = b[n];
        }
    }
    return d;
}

void __aeabi_memclr(void *d, size_t n)
{
    memset(d, 0, n);
}

void __aeabi_memclr4(void *d, size_t n)
{
    memset(d, 0, n);
}

void __aeabi_memclr8(void *d, size_t n)
{
    memset(d, 0, n);
}

void __aeabi_memcpy(void *d, const void *s, size_t n)
{
    memcpy(d, s, n);
}

void __aeabi_memcpy4(void *d, const void *s, size_t n)
{
    memcpy(d, s, n);
}

void __aeabi_memcpy8(void *d, const void *s, size_t n)
{
    memcpy(d, s, n);
}
