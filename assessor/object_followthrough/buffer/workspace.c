/*
 * ob_workspace: the variable-length-array part of the buffer laboratory.
 *
 * Two builds share one interface. The VLA build (default) declares a local
 * variable-length array; OB_NO_VLA, or a compiler defining __STDC_NO_VLA__,
 * selects the build that fills caller-provided storage instead and declares
 * no VLA at all.
 */
#include "buffer.h"

enum
{
    OB_WORKSPACE_MAX = 16 /* elements; largest request admitted */
};

#if !defined(OB_NO_VLA) && !defined(__STDC_NO_VLA__)
/* Returns n and counts the call, so the caller can see how many times the
 * array-size expression was evaluated. */
static size_t once(size_t n, unsigned *evaluations)
{
    ++*evaluations;
    return n;
}

/* n is the number of elements the callee may read; `static` states that
 * the argument must provide at least that many. The array parameter is
 * adjusted to a pointer, so no run-time bound check is implied. */
static int parameter_sum(size_t n, int values[static n])
{
    int sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += values[i];
    }
    return sum;
}
#endif

int ob_workspace(int requested, int *storage, size_t capacity, int *result,
                 unsigned *evaluations)
{
    /* Admission happens before any array is declared: a VLA of non-positive
     * or unbounded size would be undefined behaviour. */
    if (storage == NULL || result == NULL || evaluations == NULL ||
        requested <= 0 || requested > OB_WORKSPACE_MAX ||
        (size_t)requested > capacity)
    {
        return -1;
    }
    *evaluations = 0;
#if !defined(OB_NO_VLA) && !defined(__STDC_NO_VLA__)
    {
        int values[once((size_t)requested, evaluations)];
        size_t n = sizeof values / sizeof values[0];
        for (size_t i = 0; i < n; ++i)
        {
            values[i] = (int)i + 1;
        }
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
