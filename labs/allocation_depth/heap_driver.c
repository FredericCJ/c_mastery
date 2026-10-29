#include "allocation.h"
#include <stdio.h>

int main(void)
{
    struct al_heap h;
    struct al_handle handles[12] = {{0}};
    unsigned cap, policy, defer;
    if (scanf("%u%u%u", &cap, &policy, &defer) != 3 ||
        al_heap_init(&h, cap, policy, defer, 1))
    {
        return 2;
    }
    unsigned op, index, size, fail;
    while (scanf("%u%u%u%u", &op, &index, &size, &fail) == 4)
    {
        if (index >= 12)
        {
            return 2;
        }
        struct al_handle out = {0};
        int s = op == 0   ? al_heap_take(&h, size, 8, &out)
                : op == 1 ? al_heap_release(&h, handles[index])
                : op == 2 ? al_heap_resize(&h, handles[index], size, (int)fail,
                                           SIZE_MAX, &out)
                          : al_heap_collect(&h);
        if (!s && (op == 0 || op == 2))
        {
            handles[index] = out;
        }
        if (!s && op == 1)
        {
            handles[index] = (struct al_handle){0};
        }
        printf("%d", s);
        for (unsigned i = 0; i < 12; ++i)
        {
            unsigned char *p;
            size_t n;
            int live = al_heap_view(&h, handles[i], &p, &n) == AL_OK;
            if (live)
            {
                printf(" %zu:%zu", (size_t)(p - h.bytes), n);
            }
            else
            {
                printf(" -");
            }
        }
        printf(" | %zu %zu %u", h.payload, h.charged, h.free_count);
        for (unsigned i = 0; i < h.free_count; ++i)
        {
            unsigned at = h.free_offsets[i];
            printf(" %u:%u", at,
                   (unsigned)h.bytes[at] + 256u * h.bytes[at + 1]);
        }
        puts("");
        if (al_heap_check(&h))
        {
            return 3;
        }
    }
    return 0;
}
