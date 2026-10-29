#include "allocation.h"

int al_buddy_init(struct al_buddy *b)
{
    if (!b)
        return AL_BAD;
    *b = (struct al_buddy){0};
    return AL_OK;
}

static int valid(const struct al_buddy *b, unsigned node, unsigned size)
{
    if (b->state[node] > 2)
        return 0;
    if (b->state[node] == 1)
    {
        if (size == 8 || b->ids[node])
            return 0;
        return valid(b, node * 2 + 1, size / 2) &&
               valid(b, node * 2 + 2, size / 2);
    }
    if ((b->state[node] == 2) != (b->ids[node] != 0))
        return 0;
    if (size > 8)
    {
        unsigned l = node * 2 + 1, r = l + 1;
        if (b->state[l] || b->state[r] || b->ids[l] || b->ids[r])
            return 0;
        return valid(b, l, size / 2) && valid(b, r, size / 2);
    }
    return 1;
}

int al_buddy_check(const struct al_buddy *b)
{
    return b && valid(b, 0, 64) ? AL_OK : AL_CORRUPT;
}

static int take(struct al_buddy *b, unsigned node, unsigned size, unsigned off,
                unsigned need, unsigned *chosen, unsigned *offset,
                unsigned *charged)
{
    ++b->steps;
    if (b->state[node] == 2)
        return 0;
    if (size == need)
    {
        if (b->state[node])
            return 0;
        b->state[node] = 2;
        b->ids[node] = ++b->serial;
        *chosen = node;
        *offset = off;
        *charged = size;
        return 1;
    }
    b->state[node] = 1;
    if (take(b, node * 2 + 1, size / 2, off, need, chosen, offset, charged) ||
        take(b, node * 2 + 2, size / 2, off + size / 2, need, chosen, offset,
             charged))
        return 1;
    if (!b->state[node * 2 + 1] && !b->state[node * 2 + 2])
        b->state[node] = 0;
    return 0;
}

int al_buddy_take(struct al_buddy *b, size_t n, struct al_handle *out,
                  size_t *offset, size_t *charged)
{
    if (!b || !out || !offset || !charged || !n || n > 64)
        return AL_BAD;
    if (al_buddy_check(b))
        return AL_CORRUPT;
    if (b->serial == UINT32_MAX)
        return AL_RETIRED;
    unsigned need = 8;
    while (need < n)
        need *= 2;
    struct al_buddy t = *b;
    unsigned chosen, at, size;
    t.steps = 0;
    if (!take(&t, 0, 64, 0, need, &chosen, &at, &size))
        return AL_FULL;
    *b = t;
    *out = (struct al_handle){b, b->ids[chosen]};
    *offset = at;
    *charged = size;
    return AL_OK;
}

int al_buddy_release(struct al_buddy *b, struct al_handle h)
{
    if (!b)
        return AL_BAD;
    if (al_buddy_check(b))
        return AL_CORRUPT;
    if (h.owner != b || !h.id)
        return AL_STALE;
    unsigned node = 15;
    for (unsigned i = 0; i < 15; ++i)
        if (b->state[i] == 2 && b->ids[i] == h.id)
        {
            node = i;
            break;
        }
    if (node == 15)
        return AL_STALE;
    b->state[node] = 0;
    b->ids[node] = 0;
    b->steps = 1;
    while (node)
    {
        unsigned parent = (node - 1) / 2, l = parent * 2 + 1, r = l + 1;
        ++b->steps;
#if AL_MUTANT == 7
        if (b->state[l] && b->state[r])
            break;
#else
        if (b->state[l] || b->state[r])
            break;
#endif
        b->state[parent] = 0;
        node = parent;
    }
    return AL_OK;
}
