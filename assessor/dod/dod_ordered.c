#include "dod_ordered.h"
#include <limits.h>
#ifndef D_ORDER_MUTANT
#define D_ORDER_MUTANT 0
#endif
void d_order_init(struct d_ordered *o)
{
    *o = (struct d_ordered){0};
    d_sparse_init(&o->pool);
    o->head = o->tail = 4;
    o->version = 1;
    for (unsigned i = 0; i < 4; ++i)
    {
        o->next[i] = o->previous[i] = 4;
    }
}

static void link_write(struct d_ordered *o, unsigned *to, unsigned value)
{
    *to = value;
    ++o->link_writes;
}

int d_order_add(struct d_ordered *o, int value, struct d_handle *out)
{
    if (!o || !out)
    {
        return D_INVALID;
    }
    if (o->version == UINT_MAX)
    {
        return D_FULL;
    }
    struct d_handle h;
    int status = d_sparse_add(&o->pool, value, &h);
    if (status)
    {
        return status;
    }
    unsigned id = h.slot;
#if D_ORDER_MUTANT == 1
    link_write(o, &o->previous[id], 4);
    link_write(o, &o->next[id], o->head);
    if (o->head < 4)
    {
        link_write(o, &o->previous[o->head], id);
    }
    else
    {
        link_write(o, &o->tail, id);
    }
    link_write(o, &o->head, id);
#else
    link_write(o, &o->previous[id], o->tail);
    link_write(o, &o->next[id], 4);
    if (o->tail < 4)
    {
        link_write(o, &o->next[o->tail], id);
    }
    else
    {
        link_write(o, &o->head, id);
    }
    link_write(o, &o->tail, id);
#endif
    ++o->version;
    *out = h;
    return D_OK;
}

int d_order_remove(struct d_ordered *o, struct d_handle h)
{
    if (!o)
    {
        return D_INVALID;
    }
    int value, status = d_sparse_get(&o->pool, h, &value);
    if (status)
    {
        return status;
    }
    if (o->version == UINT_MAX)
    {
        return D_FULL;
    }
    unsigned before = o->previous[h.slot], after = o->next[h.slot];
    if (before < 4)
    {
        link_write(o, &o->next[before], after);
    }
    else
    {
        link_write(o, &o->head, after);
    }
    if (after < 4)
    {
        link_write(o, &o->previous[after], before);
    }
    else
    {
        link_write(o, &o->tail, before);
    }
    link_write(o, &o->next[h.slot], 4);
    link_write(o, &o->previous[h.slot], 4);
#if D_ORDER_MUTANT != 2
    ++o->version;
#endif
    return d_sparse_remove(&o->pool, h);
}

int d_order_begin(const struct d_ordered *o, struct d_ordered_iter *out)
{
    if (!o || !out)
    {
        return D_INVALID;
    }
    *out = (struct d_ordered_iter){o, o->version, o->head};
    return D_OK;
}

int d_order_next(struct d_ordered *o, struct d_ordered_iter *it,
                 struct d_handle *handle, const int **value, unsigned *has)
{
    if (!o || !it || !handle || !value || !has)
    {
        return D_INVALID;
    }
    if (it->owner != o || it->version != o->version)
    {
        return D_STALE;
    }
    if (it->next == 4)
    {
        *has = 0;
        return D_OK;
    }
    unsigned id = it->next;
    *handle = (struct d_handle){&o->pool, id, o->pool.slots[id].generation};
    *value = &o->pool.slots[id].value;
    *has = 1;
    it->next = o->next[id];
    if (o->iteration_steps < UINT_MAX)
    {
        ++o->iteration_steps;
    }
    return D_OK;
}
