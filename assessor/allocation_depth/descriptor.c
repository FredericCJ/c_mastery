#include "allocation.h"

int al_stable_init(struct al_stable *s, struct al_heap *heap)
{
    if (!s || !heap)
        return AL_BAD;
    if (al_heap_check(heap))
        return AL_CORRUPT;
    *s = (struct al_stable){0};
    s->heap = heap;
    for (unsigned i = 0; i < 3; ++i)
        s->entries[i].generation = 1;
    return AL_OK;
}

static int entry(const struct al_stable *s, struct al_stable_handle h)
{
    return s && h.owner == s && h.index < 3 && s->entries[h.index].state == 1 &&
                   s->entries[h.index].generation == h.generation
               ? (int)h.index
               : -1;
}

int al_stable_take(struct al_stable *s, size_t n, struct al_stable_handle *out)
{
    if (!s || !out)
        return AL_BAD;
    for (unsigned i = 0; i < 3; ++i)
        if (!s->entries[i].state)
        {
            struct al_handle h;
            int status = al_heap_take(s->heap, n, 8, &h);
            if (status)
                return status;
            s->entries[i].inner = h;
            s->entries[i].state = 1;
            *out = (struct al_stable_handle){s, i, s->entries[i].generation};
            return AL_OK;
        }
    return AL_FULL;
}

int al_stable_view(struct al_stable *s, struct al_stable_handle h,
                   unsigned char **out, size_t *n)
{
    if (!out || !n)
        return AL_BAD;
    int i = entry(s, h);
    if (i < 0)
        return AL_STALE;
    return al_heap_view(s->heap, s->entries[i].inner, out, n);
}

int al_stable_resize(struct al_stable *s, struct al_stable_handle h, size_t n,
                     int fail)
{
    int i = entry(s, h);
    if (i < 0)
        return AL_STALE;
    struct al_handle next;
    int status =
        al_heap_resize(s->heap, s->entries[i].inner, n, fail, SIZE_MAX, &next);
    if (status)
        return status;
    s->entries[i].inner = next;
    return AL_OK;
}

int al_stable_release(struct al_stable *s, struct al_stable_handle h)
{
    int i = entry(s, h);
    if (i < 0)
        return AL_STALE;
    int status = al_heap_release(s->heap, s->entries[i].inner);
    if (status)
        return status;
    if (s->entries[i].generation == 3)
        s->entries[i].state = 2;
    else
    {
        ++s->entries[i].generation;
        s->entries[i].state = 0;
    }
    return AL_OK;
}
