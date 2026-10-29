#include "erase.h"
#ifndef ER_MUTANT
#define ER_MUTANT 0
#endif
int er_init(struct er_owner *o, const struct er_item *items, size_t n)
{
    if (!o || n > 12 || (!items && n))
        return ER_INVALID;
    for (size_t i = 0; i < n; ++i)
    {
        if (!items[i].id)
            return ER_INVALID;
        for (size_t j = 0; j < i; ++j)
            if (items[j].id == items[i].id)
                return ER_INVALID;
    }
    struct er_owner stage = {.count = n, .epoch = 1};
    for (size_t i = 0; i < n; ++i)
        stage.items[i] = items[i];
    *o = stage;
    return ER_OK;
}

int er_compact(int *a, size_t n, int value, size_t *out, size_t *moves)
{
    if ((!a && n) || !out || !moves)
        return ER_INVALID;
    size_t write = 0, copies = 0;
    for (size_t read = 0; read < n; ++read)
        if (a[read] != value)
        {
            if (write != read)
            {
                a[write] = a[read];
                ++copies;
            }
            ++write;
        }
    *out = write;
    *moves = copies;
    return ER_OK;
}

int er_remove_if(struct er_owner *o, er_predicate_fn predicate, void *context,
                 struct er_tail *out)
{
    if (!o || !predicate || !out || o->pending)
        return ER_INVALID;
    if (o->epoch > UINT32_MAX - 2)
        return ER_EXHAUSTED;
    unsigned remove[12] = {0};
    size_t keep = 0;
    for (size_t i = 0; i < o->count; ++i)
    {
        int result = predicate(context, &o->items[i]);
        if (result != 0 && result != 1)
            return ER_FAILED;
        remove[i] = (unsigned)result;
        keep += !result;
    }
    struct er_owner stage = *o;
    size_t good = 0, bad = keep;
    for (size_t i = 0; i < o->count; ++i)
    {
        if (remove[i])
            stage.items[bad++] = o->items[i];
        else
        {
            size_t at = ER_MUTANT == 1 ? keep - 1 - good : good;
            stage.items[at] = o->items[i];
            ++good;
        }
    }
    stage.keep = keep;
    stage.pending = 1;
    ++stage.epoch;
    *o = stage;
    *out = (struct er_tail){o, o->epoch, keep};
    return ER_OK;
}

int er_erase(struct er_owner *o, struct er_tail tail, er_drop_fn drop,
             void *context)
{
    if (!o || !drop)
        return ER_INVALID;
    if (!o->pending || tail.owner != o || tail.epoch != o->epoch ||
        tail.new_end != o->keep)
        return ER_STALE;
    for (size_t i = o->keep; i < o->count; ++i)
    {
        drop(context, o->items[ER_MUTANT == 2 ? i - o->keep : i].id);
        o->items[i] = (struct er_item){0};
    }
    o->count = o->keep;
    o->pending = 0;
    ++o->epoch;
    return ER_OK;
}

int er_clear(struct er_owner *o, er_drop_fn drop, void *context)
{
    if (!o || !drop)
        return ER_INVALID;
    for (size_t i = 0; i < o->count; ++i)
    {
        drop(context, o->items[i].id);
        o->items[i] = (struct er_item){0};
    }
    o->count = o->keep = 0;
    o->pending = 0;
    if (o->epoch < UINT32_MAX)
        ++o->epoch;
    return ER_OK;
}
