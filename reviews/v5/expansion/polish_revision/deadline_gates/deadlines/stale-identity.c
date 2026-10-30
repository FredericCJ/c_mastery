#include "collection.h"

static int before(struct tc_collection *q, size_t a, size_t b)
{
#ifdef TC_COUNT_WORK
    if (q->comparisons != UINT64_MAX)
    {
        ++q->comparisons;
    }
#endif
    return q->timers[a].deadline < q->timers[b].deadline ||
           (q->timers[a].deadline == q->timers[b].deadline && a < b);
}

static void exchange(struct tc_collection *q, size_t a, size_t b)
{
    size_t x = q->index[a];
    q->index[a] = q->index[b];
    q->index[b] = x;
    q->position[q->index[a]] = a;
    q->position[q->index[b]] = b;
}

static void repair(struct tc_collection *q, size_t at)
{
    if (at && before(q, q->index[at], q->index[(at - 1) / 2]))
    {
        while (at)
        {
            size_t parent = (at - 1) / 2;
            if (!before(q, q->index[at], q->index[parent]))
            {
                break;
            }
            exchange(q, at, parent);
            at = parent;
        }
    }
    else
    {
        /* at < count/2 implies that 2*at+1 cannot overflow. */
        while (at < q->count / 2)
        {
            size_t child = 2 * at + 1;
            if (child + 1 < q->count &&
                before(q, q->index[child + 1], q->index[child]))
            {
                ++child;
            }
            if (!before(q, q->index[child], q->index[at]))
            {
                break;
            }
            exchange(q, at, child);
            at = child;
        }
    }
}

static void insert(struct tc_collection *q, size_t slot)
{
    if (q->kind == TC_HEAP)
    {
        size_t at = q->count++;
        q->index[at] = slot;
        q->position[slot] = at;
        repair(q, at);
    }
    else if (q->kind == TC_WHEEL)
    {
        size_t bucket =
            (size_t)((q->timers[slot].deadline / q->quantum) % q->buckets);
        size_t at = q->heads[bucket], prior = SIZE_MAX;
        while (at != SIZE_MAX && before(q, at, slot))
        {
            prior = at;
            at = q->next[at];
        }
        q->position[slot] = bucket;
        q->next[slot] = at;
        q->previous[slot] = prior;
        if (prior == SIZE_MAX)
        {
            q->heads[bucket] = slot;
        }
        else
        {
            q->next[prior] = slot;
        }
        if (at != SIZE_MAX)
        {
            q->previous[at] = slot;
        }
        ++q->count;
    }
    else
    {
        ++q->count;
    }
}

static void remove_slot(struct tc_collection *q, size_t slot)
{
    --q->count;
    if (q->kind == TC_HEAP)
    {
        size_t at = q->position[slot];
        q->position[slot] = SIZE_MAX;
        if (at != q->count)
        {
            q->index[at] = q->index[q->count];
            q->position[q->index[at]] = at;
            repair(q, at);
        }
    }
    else if (q->kind == TC_WHEEL)
    {
        size_t prior = q->previous[slot], after = q->next[slot];
        if (prior == SIZE_MAX)
        {
            q->heads[q->position[slot]] = after;
        }
        else
        {
            q->next[prior] = after;
        }
        if (after != SIZE_MAX)
        {
            q->previous[after] = prior;
        }
        q->position[slot] = q->next[slot] = q->previous[slot] = SIZE_MAX;
    }
}

int tc_init(struct tc_collection *q, enum tc_kind kind, struct td_timer *timers,
            size_t capacity, size_t *index, size_t *position, size_t *next,
            size_t *previous, size_t *heads, size_t buckets, uint64_t quantum,
            unsigned limit)
{
    if (!q || kind < TC_SCAN || kind > TC_WHEEL || !timers || !capacity ||
        capacity > SIZE_MAX / sizeof(struct td_timer) || !limit ||
        limit > 65535 || (kind == TC_HEAP && (!index || !position)) ||
        (kind == TC_WHEEL &&
         (!position || !next || !previous || !heads || !buckets ||
          buckets > SIZE_MAX / sizeof(size_t) || !quantum)))
    {
        return -1;
    }
    *q = (struct tc_collection){.timers = timers,
                                .capacity = capacity,
                                .kind = kind,
                                .index = index,
                                .position = position,
                                .next = next,
                                .previous = previous,
                                .heads = heads,
                                .buckets = buckets,
                                .quantum = quantum};
    for (size_t i = 0; i < capacity; ++i)
    {
        (void)td_init(timers + i, limit);
        if (position)
        {
            position[i] = SIZE_MAX;
        }
        if (next)
        {
            next[i] = SIZE_MAX;
        }
        if (previous)
        {
            previous[i] = SIZE_MAX;
        }
    }
    if (kind == TC_WHEEL)
    {
        for (size_t b = 0; b < buckets; ++b)
        {
            heads[b] = SIZE_MAX;
        }
    }
    return 0;
}

int tc_arm(struct tc_collection *q, size_t slot, uint64_t d, uint64_t p,
           int skip, struct tc_token *out)
{
    if (!q || !out || slot >= q->capacity)
    {
        return -1;
    }
    unsigned generation;
    int was_armed = q->timers[slot].state == TD_ARMED;
    int result = td_arm(q->timers + slot, d, p, skip, &generation);
    if (result != 1)
    {
        return result;
    }
    /* Remove by stored location; td_arm has already installed the new deadline. */
    if (was_armed)
    {
        remove_slot(q, slot);
    }
    insert(q, slot);
    *out = (struct tc_token){slot, generation};
    return 1;
}

int tc_cancel(struct tc_collection *q, struct tc_token token)
{
    if (!q || token.slot >= q->capacity)
    {
        return -1;
    }
    /* wrong: ignores generation */
    int result = td_cancel(q->timers + token.slot);
    if (result == 1)
    {
        remove_slot(q, token.slot);
    }
    return result;
}

int tc_peek(struct tc_collection *q, struct tc_token *out, uint64_t *deadline)
{
    if (!q || !out || !deadline)
    {
        return -1;
    }
    size_t best = SIZE_MAX;
    if (q->kind == TC_HEAP)
    {
        if (q->count)
        {
            best = q->index[0];
        }
    }
    else if (q->kind == TC_WHEEL)
    {
        for (size_t b = 0; b < q->buckets; ++b)
        {
            size_t slot = q->heads[b];
            if (slot != SIZE_MAX && (best == SIZE_MAX || before(q, slot, best)))
            {
                best = slot;
            }
        }
    }
    else
    {
        for (size_t slot = 0; slot < q->capacity; ++slot)
        {
            if (q->timers[slot].state == TD_ARMED &&
                (best == SIZE_MAX || before(q, slot, best)))
            {
                best = slot;
            }
        }
    }
    if (best == SIZE_MAX)
    {
        return 0;
    }
    *out = (struct tc_token){best, q->timers[best].generation};
    *deadline = q->timers[best].deadline;
    return 1;
}

int tc_begin(struct tc_collection *q, uint64_t now, struct tc_token *token,
             struct td_dispatch *dispatch)
{
    if (!q || !token || !dispatch)
    {
        return -1;
    }
    struct tc_token selected;
    uint64_t deadline;
    int result = tc_peek(q, &selected, &deadline);
    if (result != 1 || now < deadline)
    {
        return 0;
    }
    result =
        td_begin(q->timers + selected.slot, selected.generation, now, dispatch);
    if (result == 1)
    {
        remove_slot(q, selected.slot);
        *token = selected;
    }
    return result;
}

int tc_finish(struct tc_collection *q, struct tc_token token)
{
    if (!q || token.slot >= q->capacity)
    {
        return -1;
    }
    int result = td_finish(q->timers + token.slot, token.generation);
    if (result == 1 && q->timers[token.slot].state == TD_ARMED)
    {
        insert(q, token.slot);
    }
    return result;
}
