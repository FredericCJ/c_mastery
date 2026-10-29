#include "migration.h"
#if LP_CASE == 1
/* Same public header in implementation: incompatible definition diagnosed. */
long lp_sum(int a, int b, int *out)
{
    *out = a + b;
    return 0;
}
#elif LP_CASE == 2
int no_arguments(void);

int call(void)
{
    return no_arguments(1.25f);
}
#elif LP_CASE == 3
/* Cast is analysis-only; never call through the resulting incompatible type. */
typedef int (*wrong_comparator_fn)(const void *, const void *);

wrong_comparator_fn cast_only(void)
{
    return (wrong_comparator_fn)lp_compare;
}
#elif LP_CASE == 4
implicit_return(value) int value;
{
    return value;
}
#endif
