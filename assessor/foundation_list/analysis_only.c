#include <limits.h>

/* Compile-only: no entry point, no executable golden output. */
int ft_bad_after_add(int a, int b, int *out)
{
    int sum = a + b; /* INT_MAX,1: undefined here, before a later check. */
    if (b > 0 && sum < a)
        return 0;
    *out = sum;
    return 1;
}
