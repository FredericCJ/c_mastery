/*
 * Reference structure-of-arrays store.
 *
 * Invariant: for every i < count, x[i], y[i], id[i] and label[i] describe one
 * record, and the identifiers id[0..count) are distinct and nonzero. Each
 * operation that moves a record moves all four members together. Identifiers
 * come from a counter that only increases, so an identifier is never reused
 * while the store lives. Labels are ND_LABEL_BYTES raw bytes, not strings.
 *
 * Mutant ND_MUTANT == 7 leaves labels behind when a record is removed.
 */
#include "numeric.h"
#include <limits.h>
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

static void copy_label(char *to, const char *from)
{
    for (size_t i = 0; i < ND_LABEL_BYTES; i++)
    {
        to[i] = from[i];
    }
}

int nd_insert(struct nd_store *s, double x, double y,
              const char label[ND_LABEL_BYTES], unsigned *out)
{
    if (!s || !label || !out || s->count >= ND_ITEMS || s->serial == UINT_MAX ||
        !isfinite(x) || !isfinite(y))
    {
        return ND_BAD;
    }
    size_t slot = s->count;
    s->x[slot] = x;
    s->y[slot] = y;
    s->id[slot] = ++s->serial;
    copy_label(s->label[slot], label);
    s->count++;
    *out = s->serial;
    return ND_OK;
}

/* Index of the record with this identifier, or count if there is none. */
static size_t find_index(const struct nd_store *s, unsigned id)
{
    size_t i = 0;
    while (i < s->count && s->id[i] != id)
    {
        i++;
    }
    return i;
}

int nd_remove(struct nd_store *s, unsigned id)
{
    if (!s || s->count > ND_ITEMS)
    {
        return ND_BAD;
    }
    size_t removed = find_index(s, id);
    if (removed == s->count)
    {
        return ND_BAD;
    }
    /* Close the gap: every later record moves down one index. */
    for (size_t j = removed + 1; j < s->count; j++)
    {
        s->x[j - 1] = s->x[j];
        s->y[j - 1] = s->y[j];
        s->id[j - 1] = s->id[j];
        if (ND_MUTANT != 7)
        {
            copy_label(s->label[j - 1], s->label[j]);
        }
    }
    s->count--;
    return ND_OK;
}

/*
 * order[i] names the old index that moves to index i. The permutation is
 * checked while the new arrangement is built in a copy, so a rejected order
 * leaves the store untouched. seen is a bitmask over indices 0..ND_ITEMS - 1.
 */
int nd_reorder(struct nd_store *s, const unsigned *order, size_t n)
{
    if (!s || n != s->count || n > ND_ITEMS || (!order && n))
    {
        return ND_BAD;
    }
    struct nd_store arranged = *s;
    unsigned seen = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (order[i] >= n || (seen & (1u << order[i])))
        {
            return ND_BAD;
        }
        seen |= 1u << order[i];
        size_t from = order[i];
        arranged.x[i] = s->x[from];
        arranged.y[i] = s->y[from];
        arranged.id[i] = s->id[from];
        copy_label(arranged.label[i], s->label[from]);
    }
    *s = arranged;
    return ND_OK;
}

int nd_record(const struct nd_store *s, unsigned id, struct nd_record *out)
{
    if (!s || !out || s->count > ND_ITEMS)
    {
        return ND_BAD;
    }
    size_t i = find_index(s, id);
    if (i == s->count)
    {
        return ND_BAD;
    }
    out->x = s->x[i];
    out->y = s->y[i];
    out->id = id;
    copy_label(out->label, s->label[i]);
    return ND_OK;
}
