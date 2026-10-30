#include "allocation.h"

/*
 * Freestanding entry used by strict_entry.S: exercises arena, heap and buddy
 * once and returns the number of the first failing stage (0 = all passed).
 * No hosted library is used, so the linked image has no undefined symbols.
 */
enum
{
    ARENA_BYTES = 24,
    HEAP_BYTES = 64,
    STAGE_ARENA = 1,
    STAGE_HEAP = 2,
    STAGE_BUDDY = 3,
    BUDDY_ROUNDED_REQUEST = 9, /* charged as a 16-byte buddy */
    BUDDY_ROUNDED_CHARGE = 16
};

int al_image(void)
{
    _Alignas(8) unsigned char storage[ARENA_BYTES];
    struct al_arena arena;
    struct al_handle h;
    struct al_heap heap;
    struct al_buddy buddy;
    size_t at;
    size_t charged;
    if (al_arena_init(&arena, storage, ARENA_BYTES, 8) ||
        al_arena_take(&arena, 5, 8, 0, 0, &h) || al_arena_reset(&arena))
    {
        return STAGE_ARENA;
    }
    if (al_heap_init(&heap, HEAP_BYTES, 0, 0, 1) ||
        al_heap_take(&heap, 4, 8, &h) ||
        al_heap_resize(&heap, h, 20, 0, SIZE_MAX, &h) ||
        al_heap_release(&heap, h))
    {
        return STAGE_HEAP;
    }
    if (al_buddy_init(&buddy) ||
        al_buddy_take(&buddy, BUDDY_ROUNDED_REQUEST, &h, &at, &charged) || at ||
        charged != BUDDY_ROUNDED_CHARGE || al_buddy_release(&buddy, h))
    {
        return STAGE_BUDDY;
    }
    return 0;
}
