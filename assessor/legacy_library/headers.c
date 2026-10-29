#include "headers.h"

enum lh_status lh_size(const unsigned char *p, size_t n, size_t *out)
{
    uintmax_t value = 0;
    if (!out || (!p && n))
        return LH_ARGUMENT;
    if (!n)
        return LH_SYNTAX;
    for (size_t i = 0; i < n; ++i)
        if (p[i] < '0' || p[i] > '9')
            return LH_SYNTAX;
    for (size_t i = 0; i < n; ++i)
    {
        unsigned digit = (unsigned)(p[i] - '0');
        if (value > UINTMAX_MAX / 10 ||
            (value == UINTMAX_MAX / 10 && digit > UINTMAX_MAX % 10))
            return LH_RANGE;
        value = value * 10 + digit;
    }
#ifndef LH_MUTANT_NARROW
    if (value > SIZE_MAX)
        return LH_RANGE;
#endif
#ifdef LH_MUTANT_ZERO
    if (!value)
        return LH_SYNTAX;
#endif
    *out = (size_t)value;
    return LH_OK;
}

int lh_c11_threads(int h, long e, int no, int linked)
{
#ifdef LH_MUTANT_HEADER
    (void)e;
    (void)no;
    (void)linked;
    return h != 0;
#else
    return h && e >= 201112L && !no && linked;
#endif
}
