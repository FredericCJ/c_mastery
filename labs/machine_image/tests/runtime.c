#include "machine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern uint64_t mi_hostile(const uint32_t *, size_t);

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (!strcmp(argv[1], "abi"))
    {
        assert(mi_mix(1, 2, 3, 4, 5, 6, 7, 8) == 204);
        assert(mi_mix(UINT64_MAX, 0, 0, 0, 0, 0, 0, 1) == 7);
        assert(mi_mix(0, 0, 0, 0, 0, 0, 11, 13) == 181);
    }
    else if (!strcmp(argv[1], "hostile"))
    {
        uint32_t x[] = {1, UINT32_MAX, 3, 7};
        assert(mi_hostile(x, 4) == (uint64_t)UINT32_MAX + 11);
        assert(mi_hostile(0, 0) == 0);
        assert(mi_hostile(x + 1, 1) == UINT32_MAX);
    }
    else if (!strcmp(argv[1], "bytes"))
    {
        unsigned char a[] = {1, 2, 3, 4, 5, 6};
        assert(mi_move(a + 1, a, 5) == a + 1);
        unsigned char expected[] = {1, 1, 2, 3, 4, 5};
        assert(!memcmp(a, expected, 6));
        mi_move(a, a + 1, 5);
        assert(!memcmp(a, expected + 1, 5));
        assert(mi_move(a, a, 6) == a);
        assert(mi_move(0, 0, 0) == 0);
        assert(mi_compare("a", "b", 1) < 0);
        unsigned char high[] = {255}, low[] = {1};
        assert(mi_compare(high, low, 1) > 0);
        assert(mi_compare(0, 0, 0) == 0);
    }
    else if (!strcmp(argv[1], "wide"))
    {
        mi_u128 top = (mi_u128)1 << 127;
        assert(mi_divide(top, 2) == top / 2);
        assert(mi_divide(top + 7, top) == 1);
        assert(mi_divide((mi_u128)-1, 3) == ((mi_u128)-1) / 3);
        assert(mi_divide(99, 0) == 0);
        assert(mi_divide(17, 19) == 0);
        for (unsigned n = 0; n < 1000; n += 7)
        {
            for (unsigned d = 1; d < 71; d += 3)
            {
                assert(mi_divide(n, d) == n / d);
            }
        }
    }
    else
    {
        return 2;
    }
    puts("mechanism gate passed; full project evidence remains");
    return 0;
}
