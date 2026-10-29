#include "machine.h"

mi_u128 mi_divide(mi_u128 n, mi_u128 d)
{
    if (!d)
        return 0;
    mi_u128 q = 0, r = 0;
    for (unsigned i = 128; i > 0; --i)
    {
        unsigned k = i - 1;
        unsigned carry = (unsigned)(r >> 127);
        r = (r << 1) | ((n >> k) & 1);
        if (carry || r >= d)
        {
            r -= d;
            q |= (mi_u128)1 << k;
        }
    }
    return q;
}
