#include "list.h"
#include <stdio.h>

int main(void)
{
    for (unsigned mode = 0; mode < 2; mode++)
        for (unsigned reader = 0; reader < 2; reader++)
            for (unsigned mask = 0; mask < 32; mask++)
            {
                unsigned bits = 0;
                for (unsigned i = 0; i < 5; i++)
                    bits += (mask >> i) & 1u;
                if (bits != 2)
                    continue;
                struct sx_list l;
                struct sx_participant p[2];
                struct sx_observed o;
                struct sx_protected r;
                unsigned rd = 0, wr = 0, value = 999, count = 999, discard;
                int a = -1, b = -1, c = -1;
                if (sx_list_init(&l, mode) != MB_OK)
                    return 2;
                for (unsigned i = 0; i < 2; i++)
                    if (sx_register(&l, &p[i]) != MB_OK)
                        return 2;
                if (sx_append(&l, 11) != MB_OK ||
                    sx_enter(&l, p[reader]) != MB_OK)
                    return 2;
                for (unsigned i = 0; i < 5; i++)
                    if (mask & (1u << i))
                    {
                        if (wr++ == 0)
                        {
                            if (sx_remove(&l, &discard) != MB_OK)
                                return 2;
                        }
                        else if (sx_collect(&l, &count) != MB_OK)
                            return 2;
                    }
                    else
                    {
                        if (rd == 0)
                            a = sx_observe(&l, p[reader], 0, &o);
                        if (rd == 1 && a == MB_OK)
                            b = sx_protect(&l, p[reader], 0, o, &r);
                        if (rd == 2 && b == MB_OK)
                            c = sx_read(&l, p[reader], r, &value);
                        rd++;
                    }
                printf("H %u %u %u %d %d %d %u %u\n", mode, reader, mask, a, b,
                       c, value, count);
            }
    return 0;
}
