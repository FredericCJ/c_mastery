#include "allocation.h"

/*
 * Stable-handle table over an al_heap. An entry cycles
 * FREE -> LIVE -> FREE with a new generation each time; once it has used its
 * last generation it becomes RETIRED forever, so a stale (index, generation)
 * pair can never name a later object.
 */
enum entry_state
{
    ENTRY_FREE = 0,
    ENTRY_LIVE = 1,
    ENTRY_RETIRED = 2
};

enum
{
    FIRST_GENERATION = 1,
    LAST_GENERATION = 3,
    ALLOCATION_ALIGNMENT = 8
};

int al_stable_init(struct al_stable *s, struct al_heap *heap)
{
    if (!s || !heap)
    {
        return AL_BAD;
    }
    if (al_heap_check(heap))
    {
        return AL_CORRUPT;
    }
    *s = (struct al_stable){0};
    s->heap = heap;
    for (unsigned i = 0; i < AL_STABLE_ENTRIES; ++i)
    {
        s->entries[i].generation = FIRST_GENERATION;
    }
    return AL_OK;
}

/* Table index named by `h`, or -1 if foreign, out of range, free or stale. */
static int entry(const struct al_stable *s, struct al_stable_handle h)
{
    if (!s || h.owner != s || h.index >= AL_STABLE_ENTRIES)
    {
        return -1;
    }
    if (s->entries[h.index].state != ENTRY_LIVE ||
        s->entries[h.index].generation != h.generation)
    {
        return -1;
    }
    return (int)h.index;
}

int al_stable_take(struct al_stable *s, size_t n, struct al_stable_handle *out)
{
    if (!s || !out)
    {
        return AL_BAD;
    }
    for (unsigned i = 0; i < AL_STABLE_ENTRIES; ++i)
    {
        if (s->entries[i].state != ENTRY_FREE)
        {
            continue;
        }
        struct al_handle inner;
        int status = al_heap_take(s->heap, n, ALLOCATION_ALIGNMENT, &inner);
        if (status)
        {
            return status;
        }
        s->entries[i].inner = inner;
        s->entries[i].state = ENTRY_LIVE;
        *out = (struct al_stable_handle){s, i, s->entries[i].generation};
        return AL_OK;
    }
    return AL_FULL;
}

int al_stable_view(struct al_stable *s, struct al_stable_handle h,
                   unsigned char **out, size_t *n)
{
    if (!out || !n)
    {
        return AL_BAD;
    }
    int i = entry(s, h);
    if (i < 0)
    {
        return AL_STALE;
    }
    return al_heap_view(s->heap, s->entries[i].inner, out, n);
}

/* The client's (index, generation) is unchanged; only `inner` is replaced. */
int al_stable_resize(struct al_stable *s, struct al_stable_handle h, size_t n,
                     int fail)
{
    int i = entry(s, h);
    if (i < 0)
    {
        return AL_STALE;
    }
    struct al_handle next;
    int status =
        al_heap_resize(s->heap, s->entries[i].inner, n, fail, SIZE_MAX, &next);
    if (status)
    {
        return status;
    }
    s->entries[i].inner = next;
    return AL_OK;
}

int al_stable_release(struct al_stable *s, struct al_stable_handle h)
{
    int i = entry(s, h);
    if (i < 0)
    {
        return AL_STALE;
    }
    int status = al_heap_release(s->heap, s->entries[i].inner);
    if (status)
    {
        return status;
    }
    if (s->entries[i].generation == LAST_GENERATION)
    {
        s->entries[i].state = ENTRY_RETIRED;
    }
    else
    {
        ++s->entries[i].generation;
        s->entries[i].state = ENTRY_FREE;
    }
    return AL_OK;
}
