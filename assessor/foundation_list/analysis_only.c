/* CM-SPECIMEN: signed overflow checked after the addition has executed.
 *
 * Compile-only: no entry point, no executable golden output. */
#include <limits.h>

int ft_bad_after_add(int a, int b, int *out)
{
    /* For a = INT_MAX and b = 1 the addition itself is undefined behaviour,
     * so the later comparison cannot repair it. The correct form compares
     * a with INT_MAX - b before adding. */
    int sum = a + b;

    if (b > 0 && sum < a)
    {
        return 0;
    }
    *out = sum;
    return 1;
}
