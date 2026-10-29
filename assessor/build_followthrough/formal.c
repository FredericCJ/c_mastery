#include "formal.h"
#include <limits.h>
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
enum bf_status bf_sum(const unsigned *a, size_t extent, size_t n, unsigned *out)
{
    unsigned sum = 0;
    if (!out || n > 4 || n > extent || (n && !a))
    {
        return BF_INVALID;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (a[i] > 10)
        {
            return BF_RANGE;
        }
        sum += a[i];
    }
    if (BF_MUTANT == 1)
    {
        sum *= 2;
    }
    *out = sum;
    if (BF_MUTANT == 2)
    {
        out[1] = 0; /* Only exercised with an actual neighbor object. */
    }
    return BF_OK;
}

enum bf_status bf_sum_pairs(const unsigned *a, size_t extent, size_t n,
                            unsigned *out)
{
    unsigned sum = 0;
    size_t i = 0;
    if (!out || n > 4 || n > extent || (n && !a))
    {
        return BF_INVALID;
    }
    for (size_t j = 0; j < n; ++j)
    {
        if (a[j] > 10)
        {
            return BF_RANGE;
        }
    }
    for (; n - i >= 2; i += 2)
    {
        sum += a[i] + a[i + 1];
    }
    if (i < n)
    {
        sum += a[i];
    }
    *out = sum;
    return BF_OK;
}

enum bf_status bf_signed(const int *a, size_t extent, size_t n, int *out)
{
    int sum = 0;
    if (!out || n > 4 || n > extent || (n && !a))
    {
        return BF_INVALID;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if ((a[i] > 0 && sum > INT_MAX - a[i]) ||
            (a[i] < 0 && sum < INT_MIN - a[i]))
        {
            return BF_RANGE;
        }
        sum += a[i];
    }
    *out = sum;
    return BF_OK;
}

enum bf_status bf_max(const unsigned *a, size_t extent, size_t n, unsigned *out)
{
    unsigned m = 0;
    if (!out || n > 4 || n > extent || (n && !a))
    {
        return BF_INVALID;
    }
    if (!n)
    {
        return BF_EMPTY;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (a[i] > 10)
        {
            return BF_RANGE;
        }
        if (a[i] > m)
        {
            m = a[i];
        }
    }
    *out = m;
    return BF_OK;
}

enum bf_status bf_write(bf_writer writer, void *ctx, const unsigned char *a,
                        size_t n, unsigned budget, size_t *committed)
{
    size_t done = 0;
    if (!writer || !committed || (n && !a))
    {
        return BF_INVALID;
    }
    *committed = 0;
    for (unsigned calls = 0; done < n && calls < budget; ++calls)
    {
        ptrdiff_t got = writer(ctx, a + done, n - done);
        if (got < 0)
        {
            return BF_IO;
        }
        if ((size_t)got > n - done)
        {
            return BF_INVALID;
        }
        if (BF_MUTANT == 3 && got > 0)
        {
            done = n;
        }
        else
        {
            done += (size_t)got;
        }
        *committed = done;
    }
    return done == n ? BF_OK : BF_PENDING;
}

unsigned bf_read_clear(unsigned *status)
{
    unsigned before = *status;
    *status &= ~1u;
    return before;
}
