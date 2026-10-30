#include "allocation.h"
#include <stdio.h>

/*
 * Replays a trace given as argv[1] over a 64-byte heap and prints the result
 * of al_heap_check. Trace letters: 'a' allocate one 4-byte object (if none is
 * live), 'f' free it, 'c' corrupt the last byte of the final block's footer,
 * which is also the last byte of the heap.
 *
 * Exit status: 0 after printing the check result, 2 for bad usage or an
 * unknown letter, 3 if allocate or free fails on a healthy heap.
 */
enum
{
    HEAP_BYTES = 64,
    OBJECT_BYTES = 4,
    ALIGNMENT = 8,
    FOOTER_CANARY_OFFSET = 63, /* last byte of the final block's footer */
    EXIT_BAD_USAGE = 2,
    EXIT_ALLOCATOR_ERROR = 3
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return EXIT_BAD_USAGE;
    }
    struct al_heap h;
    struct al_handle live = {0};
    if (al_heap_init(&h, HEAP_BYTES, 0, 0, 0))
    {
        return EXIT_ALLOCATOR_ERROR;
    }
    for (const char *p = argv[1]; *p; ++p)
    {
        if (*p == 'a')
        {
            if (!live.id && al_heap_take(&h, OBJECT_BYTES, ALIGNMENT, &live))
            {
                return EXIT_ALLOCATOR_ERROR;
            }
        }
        else if (*p == 'f')
        {
            if (live.id && al_heap_release(&h, live))
            {
                return EXIT_ALLOCATOR_ERROR;
            }
            live.id = 0;
        }
        else if (*p == 'c')
        {
            h.bytes[FOOTER_CANARY_OFFSET] ^= 1;
        }
        else
        {
            return EXIT_BAD_USAGE;
        }
    }
    printf("%d\n", al_heap_check(&h));
    return 0;
}
