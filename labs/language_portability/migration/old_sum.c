/* Original teaching source with an identifier-list definition; C90 experiment. */
#include <limits.h>
int lp_old_sum(a, b, out)
int a;
int b;
int *out;
{
    if (!out)
        return 1;
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return 2;
    *out = a + b;
    return 0;
}
