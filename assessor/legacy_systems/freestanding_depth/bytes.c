#include "free.h"

void *fr_copy(void *destination, const void *source, size_t n)
{
    unsigned char *d = destination;
    const unsigned char *s = source;
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = s[i];
    }
    return destination;
}

void *fr_move(void *destination, const void *source, size_t n)
{
    unsigned char *d = destination;
    const unsigned char *s = source;
#ifdef FR_MUTANT_FORWARD
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = s[i];
    }
#else
    if ((uintptr_t)d < (uintptr_t)s)
    {
        for (size_t i = 0; i < n; ++i)
        {
            d[i] = s[i];
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
#endif
    return destination;
}

void *fr_set(void *destination, unsigned char value, size_t n)
{
    unsigned char *d = destination;
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = value;
    }
    return destination;
}

int fr_compare(const void *left, const void *right, size_t n)
{
    const unsigned char *a = left, *b = right;
    for (size_t i = 0; i < n; ++i)
    {
#ifdef FR_MUTANT_SIGNED
        if ((signed char)a[i] != (signed char)b[i])
        {
            return (signed char)a[i] < (signed char)b[i] ? -1 : 1;
        }
#else
        if (a[i] != b[i])
        {
            return a[i] < b[i] ? -1 : 1;
        }
#endif
    }
    return 0;
}

static enum fr_status admission(unsigned char *a, size_t capacity, size_t d,
                                size_t s, size_t n)
{
    if (!a && capacity)
    {
        return FR_ARGUMENT;
    }
    if (d > capacity || s > capacity || n > capacity - d || n > capacity - s)
    {
        return FR_RANGE;
    }
    return FR_OK;
}

enum fr_status fr_arena_copy(unsigned char *a, size_t capacity, size_t d,
                             size_t s, size_t n)
{
    enum fr_status rc = admission(a, capacity, d, s, n);
    if (rc != FR_OK)
    {
        return rc;
    }
    if (n && (d > s ? d - s < n : s - d < n))
    {
        return FR_OVERLAP;
    }
    for (size_t i = 0; i < n; ++i)
    {
        a[d + i] = a[s + i];
    }
    return FR_OK;
}

enum fr_status fr_arena_move(unsigned char *a, size_t capacity, size_t d,
                             size_t s, size_t n)
{
    enum fr_status rc = admission(a, capacity, d, s, n);
    if (rc != FR_OK)
    {
        return rc;
    }
    if (d < s)
    {
        for (size_t i = 0; i < n; ++i)
        {
            a[d + i] = a[s + i];
        }
    }
    else
    {
        while (n)
        {
            --n;
            a[d + n] = a[s + n];
        }
    }
    return FR_OK;
}
