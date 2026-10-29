#include <stddef.h>

/* Original target byte helper, ISO C comparison sign only. No null/count-zero
   extension is part of the caller contract; all callers pass live arrays. */
int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; i++)
    {
        if (x[i] < y[i])
        {
            return -1;
        }
        if (x[i] > y[i])
        {
            return 1;
        }
    }
    return 0;
}
