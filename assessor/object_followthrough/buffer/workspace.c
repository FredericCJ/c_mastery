#include "buffer.h"
#if !defined(OB_NO_VLA) && !defined(__STDC_NO_VLA__)
static size_t once(size_t n, unsigned *evaluations)
{
    ++*evaluations;
    return n;
}

static int parameter_sum(size_t n, int values[static n])
{
    int sum = 0;
    for (size_t i = 0; i < n; ++i)
        sum += values[i];
    return sum;
}
#endif
int ob_workspace(int requested, int *storage, size_t capacity, int *result,
                 unsigned *evaluations)
{
    if (!storage || !result || !evaluations || requested <= 0 ||
        requested > 16 || (size_t)requested > capacity)
        return -1;
    *evaluations = 0;
#if !defined(OB_NO_VLA) && !defined(__STDC_NO_VLA__)
    {
        int values[once((size_t)requested, evaluations)];
        size_t n = sizeof values / sizeof values[0];
        for (size_t i = 0; i < n; ++i)
            values[i] = (int)i + 1;
        *result = parameter_sum(n, values);
    }
#else
    {
        size_t n = (size_t)requested;
        int sum = 0;
        for (size_t i = 0; i < n; ++i)
        {
            storage[i] = (int)i + 1;
            sum += storage[i];
        }
        *result = sum;
    }
#endif
    return 0;
}
