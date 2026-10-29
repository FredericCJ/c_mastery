#include "dispatch_dictionary.h"
#include <limits.h>
#ifndef DS_REDUCTION_MUTANT
#define DS_REDUCTION_MUTANT 0
#endif
int ds_sat_add(int a, int b, int *out)
{
    if (!out)
        return DS_INVALID;
    if (b > 0 && a > INT_MAX - b)
        *out = DS_REDUCTION_MUTANT == 1 ? INT_MAX - 1 : INT_MAX;
    else if (b < 0 && a < INT_MIN - b)
        *out = INT_MIN;
    else
        *out = a + b;
    return DS_OK;
}

int ds_sat_left(const int *p, size_t n, int *out)
{
    if (!out || (!p && n) || n > 16)
        return DS_INVALID;
    int result = 0;
    for (size_t i = 0; i < n; ++i)
        (void)ds_sat_add(result, p[i], &result);
    *out = result;
    return DS_OK;
}

static int tree(const int *p, size_t n)
{
    if (!n)
        return 0;
    if (n == 1)
        return *p;
    size_t half = n / 2;
    int a = tree(p, half), b = tree(p + half, n - half), out;
    (void)ds_sat_add(a, b, &out);
    return out;
}

int ds_sat_tree(const int *p, size_t n, int *out)
{
    if (!out || (!p && n) || n > 16)
        return DS_INVALID;
    if (DS_REDUCTION_MUTANT == 2)
        return ds_sat_left(p, n, out);
    *out = tree(p, n);
    return DS_OK;
}
