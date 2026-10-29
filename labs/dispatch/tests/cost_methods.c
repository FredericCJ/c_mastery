#include "dispatch.h"
#include <limits.h>

int dc_add(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
    {
        return DS_RANGE;
    }
    *out = a + b;
    return DS_OK;
}

int dc_max(int a, int b, int *out)
{
    *out = a > b ? a : b;
    return DS_OK;
}

ds_combine_fn dc_choose(unsigned selector)
{
    return selector & 1 ? dc_max : dc_add;
}

int dc_direct(const int *p, int *out)
{
    if (!p || !out)
    {
        return DS_INVALID;
    }
    *out = *p;
    return DS_OK;
}

int dc_measure(const void *p, int *out)
{
    *out = *(const int *)p;
    return DS_OK;
}

int dc_max_fold(const int *p, size_t n, int initial, int *out)
{
    if (!out || (!p && n) || n > 16)
    {
        return DS_INVALID;
    }
    int value = initial;
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i] > value)
        {
            value = p[i];
        }
    }
    *out = value;
    return DS_OK;
}
