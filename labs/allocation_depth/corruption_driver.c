#include "allocation.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    struct al_heap h;
    struct al_handle live = {0};
    if (al_heap_init(&h, 64, 0, 0, 0))
        return 3;
    for (const char *p = argv[1]; *p; ++p)
    {
        if (*p == 'a')
        {
            if (!live.id && al_heap_take(&h, 4, 8, &live))
                return 3;
        }
        else if (*p == 'f')
        {
            if (live.id && al_heap_release(&h, live))
                return 3;
            live.id = 0;
        }
        else if (*p == 'c')
            h.bytes[63] ^= 1;
        else
            return 2;
    }
    printf("%d\n", al_heap_check(&h));
    return 0;
}
