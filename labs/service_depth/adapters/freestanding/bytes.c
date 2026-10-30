#include <stddef.h>
#include <stdint.h>

/* Original byte loops for the guest closure: the compiler may emit calls to
 * these even with -ffreestanding. `volatile` stops it from recognizing the
 * loop and turning it back into a call to the function being defined. */
void *memcpy(void *to, const void *from, size_t n)
{
    volatile unsigned char *dst = to;
    const volatile unsigned char *src = from;
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = src[i];
    }
    return to;
}

void *memset(void *to, int value, size_t n)
{
    volatile unsigned char *dst = to;
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = (unsigned char)value;
    }
    return to;
}

int memcmp(const void *left, const void *right, size_t n)
{
    const volatile unsigned char *a = left;
    const volatile unsigned char *b = right;
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
        {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    return 0;
}

void *memmove(void *to, const void *from, size_t n)
{
    volatile unsigned char *dst = to;
    const volatile unsigned char *src = from;
    /* Copy forward when the destination is below the source and backward
     * otherwise, so overlapping ranges are not clobbered. Comparing the
     * addresses is only meaningful on this flat-address target; it is not a
     * portable ordering of unrelated objects. */
    if ((uintptr_t)to < (uintptr_t)from)
    {
        for (size_t i = 0; i < n; i++)
        {
            dst[i] = src[i];
        }
    }
    else
    {
        while (n)
        {
            n--;
            dst[n] = src[n];
        }
    }
    return to;
}
