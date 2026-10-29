#include "command.h"
#include <stdio.h>
_Static_assert(UINT_MAX == 4294967295u,
               "Host oracle protocol uses 32-bit unsigned");

int main(void)
{
    unsigned ext, allowed, generation, n;
    while (scanf("%u%u%u%u", &ext, &allowed, &generation, &n) == 4)
    {
        unsigned char bytes[40];
        unsigned i, j, value;
        struct sc_batch b;
        struct sc_meter m;
        struct sc_authority a = {generation, allowed};
        enum sc_result r;
        if (n > sizeof bytes)
            return 2;
        for (i = 0; i < n; ++i)
        {
            if (scanf("%u", &value) != 1 || value > 255)
                return 2;
            bytes[i] = (unsigned char)value;
        }
        r = sc_decode(bytes, n, &a, (int)ext, &b, &m);
        printf("%u", (unsigned)r);
        if (r == SC_OK)
        {
            printf(" %u %u", b.count, b.work);
            for (i = 0; i < b.count; ++i)
            {
                const struct sc_command *c = &b.commands[i];
                printf(" %u %u %u", c->opcode, c->count, c->generation);
                for (j = 0; j < c->count; ++j)
                    printf(" %u", c->values[j]);
            }
        }
        putchar('\n');
    }
    return ferror(stdin) ? 2 : 0;
}
