#include "allocation.h"

/*
 * Arithmetic for placing `n` bytes at alignment `a` (a power of two) after
 * `c`. Padding is the least nonnegative amount that makes the offset a
 * multiple of `a`: (0 - c) & (a - 1) in unsigned arithmetic. It is compared
 * against the remaining capacity in two steps so that no sum can wrap.
 */
int al_extent(size_t c, size_t n, size_t a, size_t cap, size_t *at, size_t *end)
{
    if (!at || !end || !n || !a || (a & (a - 1)) || c > cap)
    {
        return AL_BAD;
    }
    size_t padding = (0 - c) & (a - 1);
#if AL_MUTANT == 1
    padding = 0;
#endif
    size_t remaining = cap - c;
    if (padding > remaining || n > remaining - padding)
    {
        return AL_FULL;
    }
    *at = c + padding;
    *end = *at + n;
    return AL_OK;
}

static int is_power_of_two(size_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

int al_arena_init(struct al_arena *a, unsigned char *p, size_t cap,
                  size_t align)
{
    if (!a || (!p && cap) || !is_power_of_two(align))
    {
        return AL_BAD;
    }
    *a = (struct al_arena){0};
    a->bytes = p;
    a->capacity = cap;
    a->alignment = align;
    return AL_OK;
}

static void note_peak(struct al_arena *a, size_t cursor)
{
    if (cursor > a->peak)
    {
        a->peak = cursor;
    }
    if (cursor > a->epoch_peak)
    {
        a->epoch_peak = cursor;
    }
}

/*
 * Check order is part of the contract: argument errors, then space (AL_FULL
 * from al_extent), then item-table capacity, then serial exhaustion. Nothing
 * is modified before the last check passes.
 */
int al_arena_take(struct al_arena *a, size_t n, size_t align, al_cleanup_fn f,
                  void *ctx, struct al_handle *out)
{
    if (!a || !out || align > a->alignment)
    {
        return AL_BAD;
    }
    size_t at;
    size_t end;
    int status = al_extent(a->used, n, align, a->capacity, &at, &end);
    if (status)
    {
        return status;
    }
    if (a->count == AL_ARENA_ITEMS)
    {
        return AL_FULL;
    }
    if (a->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    struct al_item item = {at, n, a->used, ++a->serial, f, ctx};
    a->items[a->count++] = item;
    a->used = end;
    a->payload += n;
    note_peak(a, end);
    *out = (struct al_handle){a, item.id};
    return AL_OK;
}

/* Position of the live item named by `h`, or -1 for a foreign/stale handle. */
static int index_of(const struct al_arena *a, struct al_handle h)
{
    if (!a || h.owner != a || !h.id)
    {
        return -1;
    }
    for (unsigned i = 0; i < a->count; ++i)
    {
        if (a->items[i].id == h.id)
        {
            return (int)i;
        }
    }
    return -1;
}

int al_arena_view(struct al_arena *a, struct al_handle h, unsigned char **out,
                  size_t *size)
{
    if (!out || !size)
    {
        return AL_BAD;
    }
    int i = index_of(a, h);
    if (i < 0)
    {
        return AL_STALE;
    }
    *out = a->bytes + a->items[i].offset;
    *size = a->items[i].size;
    return AL_OK;
}

int al_arena_mark(struct al_arena *a, struct al_mark *out)
{
    if (!a || !out)
    {
        return AL_BAD;
    }
    if (a->mark_count == AL_ARENA_MARKS)
    {
        return AL_FULL;
    }
    if (a->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    unsigned i = a->mark_count++;
    a->marks[i].id = ++a->serial;
    a->marks[i].count = a->count;
    a->marks[i].used = a->used;
    *out = (struct al_mark){a, a->serial};
    return AL_OK;
}

/* Marks nest: only the newest mark of this arena may be used. */
static int is_newest_mark(const struct al_arena *a, struct al_mark mark)
{
    return a && a->mark_count && mark.owner == a &&
           mark.id == a->marks[a->mark_count - 1].id;
}

/*
 * Pops items newest-first until `count` remain, restoring the cursor and
 * payload for each. The item is reclaimed BEFORE its cleanup runs, and a
 * failing cleanup does not stop the remaining ones (no destructor is skipped
 * or repeated). Returns AL_CLEANUP if any cleanup failed.
 */
static int pop_items_to(struct al_arena *a, unsigned count)
{
    int failed = 0;
    while (a->count > count)
    {
        struct al_item item = a->items[--a->count];
        a->used = item.before;
        a->payload -= item.size;
        if (item.cleanup && item.cleanup(item.context))
        {
            failed = 1;
        }
    }
    return failed ? AL_CLEANUP : AL_OK;
}

int al_arena_rollback(struct al_arena *a, struct al_mark mark)
{
    if (!is_newest_mark(a, mark))
    {
        return AL_STALE;
    }
    unsigned i = --a->mark_count;
    int status = pop_items_to(a, a->marks[i].count);
    a->used = a->marks[i].used;
    return status;
}

/* The items stay; their cleanup duty passes to the enclosing mark or reset. */
int al_arena_commit(struct al_arena *a, struct al_mark mark)
{
    if (!is_newest_mark(a, mark))
    {
        return AL_STALE;
    }
    --a->mark_count;
    return AL_OK;
}

int al_arena_release(struct al_arena *a, struct al_handle h)
{
    int i = index_of(a, h);
    if (i < 0)
    {
        return AL_STALE;
    }
#if AL_MUTANT != 2
    if ((unsigned)i + 1 != a->count)
    {
        return AL_BAD;
    }
#endif
    if (a->mark_count && a->count <= a->marks[a->mark_count - 1].count)
    {
        return AL_BAD;
    }
    return pop_items_to(a, (unsigned)i);
}

int al_arena_reset(struct al_arena *a)
{
    if (!a)
    {
        return AL_BAD;
    }
    int status = pop_items_to(a, 0);
    a->mark_count = 0;
    a->epoch_peak = 0;
    return status;
}
