#include "migration.h"
#include <limits.h>
#ifndef LP_MUTANT
#define LP_MUTANT 0
#endif
/* Checked a + b; the limits are tested before the add because signed overflow
   is undefined. Returns LP_INVALID (null out) or LP_RANGE, leaving *out
   unchanged, else LP_OK. */
int lp_sum(int a, int b, int *out)
{
    if (!out)
    {
        return LP_INVALID;
    }
    if (LP_MUTANT == 1)
    {
        *out = 0;
    }
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
    {
        return LP_RANGE;
    }
    *out = a + b;
    return LP_OK;
}

/* Three-way key comparison written as (a > b) - (a < b): a subtraction would
   overflow for keys of opposite sign. Both records are non-null. */
int lp_compare(const struct lp_record *a, const struct lp_record *b)
{
    if (LP_MUTANT == 2)
    {
        return (unsigned)a->key > (unsigned)b->key   ? 1
               : (unsigned)a->key < (unsigned)b->key ? -1
                                                     : 0;
    }
    return (a->key > b->key) - (a->key < b->key);
}

/* qsort()-shaped comparator over struct lp_record. */
int lp_adapter(const void *a, const void *b)
{
    return lp_compare(a, b);
}

/* qsort_s()-shaped comparator: context points to a read-only struct lp_order
   that selects descending order. */
int lp_context_adapter(const void *a, const void *b, void *context)
{
    const struct lp_order *order = context;
    int result = lp_adapter(a, b);
    return order->reverse && LP_MUTANT != 3 ? -result : result;
}

enum
{
    SORT_LIMIT = 16
};

/* Stable insertion sort of n <= 16 records; equal keys keep their order. An
   element moves left only past records that sort strictly after it, so ties
   never swap.
   Returns LP_INVALID (null array with n > 0, null order, reverse not 0/1) or
   LP_RANGE (n > 16), leaving the array unchanged. */
int lp_sort(struct lp_record *a, size_t n, const struct lp_order *order)
{
    if ((!a && n) || !order || order->reverse > 1)
    {
        return LP_INVALID;
    }
    if (n > SORT_LIMIT)
    {
        return LP_RANGE;
    }
    /* Copy the read-only order to make callback borrowing explicit. */
    struct lp_order context = *order;
    for (size_t i = 1; i < n; ++i)
    {
        struct lp_record value = a[i];
        size_t j = i;
        while (j)
        {
            int comparison = lp_context_adapter(&a[j - 1], &value, &context);
            if (comparison < 0 || (comparison == 0 && LP_MUTANT != 4))
            {
                break;
            }
            a[j] = a[j - 1];
            --j;
        }
        a[j] = value;
    }
    return LP_OK;
}
