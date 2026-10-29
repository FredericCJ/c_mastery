#include "heap.h"

static int better(uint32_t a, uint32_t b, unsigned minimum)
{
    return minimum ? a < b : a > b;
}

int ah_trace(uint32_t *a, uint32_t n, uint32_t capacity, uint32_t p,
             unsigned arity, unsigned minimum, struct ah_stats *out,
             struct ah_trace *trace)
{
    if (!out || !trace || n > capacity || n > 1024 || (n && (!a || p >= n)) ||
        (arity != 2 && arity != 3) || minimum > 1)
        return 1;
    for (unsigned i = 1; i < n; ++i)
    {
        unsigned parent = (i - 1) / arity;
        if (parent != p && better(a[i], a[parent], minimum))
            return 1;
#ifndef AH_MUTANT_PREMISE
        if (p && parent == p && better(a[i], a[(p - 1) / arity], minimum))
            return 1;
#endif
    }
    *out = (struct ah_stats){0};
    trace->used = 0;
    if (!n)
        return 0;
    while (n >= 2 && p <= (n - 2) / arity)
    {
        unsigned first = arity * p + 1, child = first;
        for (unsigned i = 1; i < arity && first + i < n; ++i)
        {
            ++out->comparisons;
            if (better(a[first + i], a[child], minimum))
                child = first + i;
        }
        ++out->comparisons;
        if (!better(a[child], a[p], minimum))
            break;
        uint32_t old = a[p];
        a[p] = a[child];
#ifdef AH_MUTANT_DUPLICATE
        a[child] = a[p];
        (void)old;
#else
        a[child] = old;
#endif
        p = child;
        unsigned at = trace->used++;
        ++out->swaps;
        trace->position[at] = p;
        for (unsigned i = 0; i < n; ++i)
            trace->state[at][i] = a[i];
    }
    return 0;
}
