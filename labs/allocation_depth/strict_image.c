#include "allocation.h"

int al_image(void)
{
    _Alignas(8) unsigned char storage[24];
    struct al_arena arena;
    struct al_handle h;
    struct al_heap heap;
    struct al_buddy buddy;
    size_t at, charged;
    if (al_arena_init(&arena, storage, 24, 8) ||
        al_arena_take(&arena, 5, 8, 0, 0, &h) || al_arena_reset(&arena))
        return 1;
    if (al_heap_init(&heap, 64, 0, 0, 1) || al_heap_take(&heap, 4, 8, &h) ||
        al_heap_resize(&heap, h, 20, 0, SIZE_MAX, &h) ||
        al_heap_release(&heap, h))
        return 2;
    if (al_buddy_init(&buddy) || al_buddy_take(&buddy, 9, &h, &at, &charged) ||
        at || charged != 16 || al_buddy_release(&buddy, h))
        return 3;
    return 0;
}
