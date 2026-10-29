#include "facade.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
struct bf_record *bf_mutable(struct bf_record *p)
{
    return p;
}

const struct bf_record *bf_const(const struct bf_record *p)
{
#if BF_MUTANT == 1
    (void)p;
    return 0;
#else
    return p;
#endif
}

int bf_binary_int(int a, int b)
{
#if BF_MUTANT == 2
    return b * 100 + a;
#else
    return a * 100 + b;
#endif
}

unsigned bf_binary_unsigned(unsigned a, unsigned b)
{
    return a * 100u + b;
}

int bf_sequence(bf_get_fn left, void *lc, bf_get_fn right, void *rc, int *out)
{
    int a, b;
    if (!left || !right || !out)
    {
        return -1;
    }
#if BF_MUTANT == 3
    b = right(rc);
    a = left(lc);
    if (a < 0 || a > 99 || b < 0 || b > 99)
    {
        return -1;
    }
#else
    a = left(lc);
    if (a < 0 || a > 99)
    {
        return -1;
    }
    b = right(rc);
    if (b < 0 || b > 99)
    {
        return -1;
    }
#endif
    *out = bf_binary_int(a, b);
    return 0;
}
