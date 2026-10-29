#include "memory.h"
#include <limits.h>
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
static int valid(struct cpx_store *s, struct cpx_handle h)
{
    return s && s->records && h.slot < s->capacity && s->state[h.slot] == 1 &&
           s->generation[h.slot] == h.generation;
}

static void dispose(struct cpx_store *s)
{
    if (s->nodes)
    {
        s->backend.release(s->backend.context, s->nodes);
        s->nodes = NULL;
    }
    if (s->records)
    {
        s->backend.release(s->backend.context, s->records);
        s->records = NULL;
    }
    s->cookie = 0;
    s->count = 0;
    s->head = s->tail = CPX_NONE;
    s->closing = 2;
}

enum cpx_status cpx_init(struct cpx_store *s, struct cpx_backend b,
                         unsigned capacity, unsigned nodes)
{
    if (!s || s->records || s->nodes || !b.allocate || !b.release ||
        !b.subscribe || !b.unsubscribe || !capacity || capacity > CPX_LIMIT ||
        !nodes || nodes > capacity)
        return CPX_INVALID;
    struct cpx_store next = {0};
    next.backend = b;
    next.capacity = capacity;
    next.node_capacity = nodes;
    next.head = next.tail = CPX_NONE;
    next.records = b.allocate(b.context, capacity * sizeof *next.records);
    if (!next.records)
        return CPX_FULL;
    next.nodes = b.allocate(b.context, nodes * sizeof *next.nodes);
    if (!next.nodes)
    {
        dispose(&next);
        return CPX_FULL;
    }
    for (unsigned i = 0; i < capacity; ++i)
    {
        next.records[i] = (struct cpx_record){0};
        next.generation[i] = 1;
    }
    for (unsigned i = 0; i < nodes; ++i)
        next.nodes[i] = (struct cpx_node){{0, 0}, CPX_NONE, 0};
    if (!b.subscribe(b.context, &next.cookie) || !next.cookie)
    {
        dispose(&next);
        return CPX_FAILED;
    }
    *s = next;
    return CPX_OK;
}

enum cpx_status cpx_close(struct cpx_store *s)
{
    if (!s)
        return CPX_INVALID;
    if (!s->records)
        return CPX_OK;
    for (unsigned i = 0; i < s->capacity; ++i)
        if (s->pins[i])
            return CPX_BUSY;
    if (s->closing)
        return CPX_PENDING;
    s->closing = 1;
    int done = s->backend.unsubscribe(s->backend.context, s->cookie);
    if (!done && CPX_MUTANT != 1)
        return CPX_PENDING;
    dispose(s);
    return CPX_OK;
}

enum cpx_status cpx_acknowledge(struct cpx_store *s, uint64_t cookie,
                                int quiescent)
{
    if (!s || !s->records || s->closing != 1 || !cookie || cookie != s->cookie)
        return CPX_INVALID;
    if (!quiescent)
        return CPX_PENDING;
    dispose(s);
    return CPX_OK;
}

enum cpx_status cpx_allocate(struct cpx_store *s,
                             const struct cpx_record *record,
                             struct cpx_handle *out)
{
    if (!s || !s->records || !record || !out || record->length > 16)
        return CPX_INVALID;
    if (s->closing)
        return CPX_BUSY;
    for (unsigned i = 0; i < s->capacity; ++i)
        if (!s->state[i])
        {
            s->records[i] = *record;
            s->state[i] = 1;
            *out = (struct cpx_handle){i, s->generation[i]};
            return CPX_OK;
        }
    return CPX_FULL;
}

enum cpx_status cpx_lookup(struct cpx_store *s, struct cpx_handle h,
                           const struct cpx_record **out)
{
    if (!out || !valid(s, h))
        return CPX_INVALID;
    *out = &s->records[h.slot];
    return CPX_OK;
}

enum cpx_status cpx_pin(struct cpx_store *s, struct cpx_handle h)
{
    if (!valid(s, h) || s->pins[h.slot] == UINT_MAX)
        return CPX_INVALID;
    ++s->pins[h.slot];
    return CPX_OK;
}

enum cpx_status cpx_unpin(struct cpx_store *s, struct cpx_handle h)
{
    if (!valid(s, h) || !s->pins[h.slot])
        return CPX_INVALID;
    --s->pins[h.slot];
    return CPX_OK;
}

enum cpx_status cpx_release(struct cpx_store *s, struct cpx_handle h)
{
    if (!valid(s, h))
        return CPX_INVALID;
    if (s->pins[h.slot])
        return CPX_BUSY;
    if (s->generation[h.slot] == CPX_GENERATION_MAX)
        s->state[h.slot] = 2;
    else
    {
        s->state[h.slot] = 0;
        ++s->generation[h.slot];
    }
    return CPX_OK;
}

enum cpx_status cpx_relocate(struct cpx_store *s)
{
    if (!s || !s->records || s->closing)
        return CPX_INVALID;
    for (unsigned i = 0; i < s->capacity; ++i)
        if (s->pins[i])
            return CPX_BUSY;
    struct cpx_record *next =
        s->backend.allocate(s->backend.context, s->capacity * sizeof *next);
    if (!next)
        return CPX_FULL;
    for (unsigned i = 0; i < s->capacity; ++i)
        next[i] = s->records[i];
    s->backend.release(s->backend.context, s->records);
    s->records = next;
    return CPX_OK;
}

enum cpx_status cpx_grow(struct cpx_store *s, unsigned capacity)
{
    if (!s || !s->records || s->closing || capacity <= s->capacity ||
        capacity > CPX_LIMIT)
        return CPX_INVALID;
    for (unsigned i = 0; i < s->capacity; ++i)
        if (s->pins[i])
            return CPX_BUSY;
    struct cpx_record *records =
        s->backend.allocate(s->backend.context, capacity * sizeof *records);
    if (!records)
        return CPX_FULL;
    struct cpx_node *nodes =
        s->backend.allocate(s->backend.context, capacity * sizeof *nodes);
    if (!nodes)
    {
        s->backend.release(s->backend.context, records);
        return CPX_FULL;
    }
    for (unsigned i = 0; i < capacity; ++i)
    {
        records[i] = i < s->capacity ? s->records[i] : (struct cpx_record){0};
        nodes[i] = i < s->node_capacity
                       ? s->nodes[i]
                       : (struct cpx_node){{0, 0}, CPX_NONE, 0};
    }
    for (unsigned i = s->capacity; i < capacity; ++i)
    {
        s->state[i] = 0;
        s->generation[i] = 1;
        s->pins[i] = 0;
    }
    s->backend.release(s->backend.context, s->nodes);
    s->backend.release(s->backend.context, s->records);
    s->records = records;
    s->nodes = nodes;
    s->capacity = s->node_capacity = capacity;
    return CPX_OK;
}

enum cpx_status cpx_push(struct cpx_store *s, struct cpx_handle h)
{
    if (!valid(s, h))
        return CPX_INVALID;
    if (s->closing)
        return CPX_BUSY;
    if (s->node_attempts == UINT_MAX)
        return CPX_FULL;
    ++s->node_attempts;
    if (s->node_fail_at == s->node_attempts)
        return CPX_FULL;
    for (unsigned i = 0; i < s->node_capacity; ++i)
        if (!s->nodes[i].used)
        {
            /* A queued node owns a pin; acquiring it precedes publishing the node. */
            if (cpx_pin(s, h) != CPX_OK)
                return CPX_FULL;
            s->nodes[i] = (struct cpx_node){h, CPX_NONE, 1};
            if (s->tail == CPX_NONE)
                s->head = i;
            else
                s->nodes[s->tail].next = i;
            s->tail = i;
            ++s->count;
            return CPX_OK;
        }
    return CPX_FULL;
}

enum cpx_status cpx_pop(struct cpx_store *s, struct cpx_handle *out)
{
    if (!s || !s->records || !out)
        return CPX_INVALID;
    if (!s->count)
        return CPX_FULL;
    unsigned node = s->head;
    *out = s->nodes[node].handle;
    s->head = s->nodes[node].next;
    s->nodes[node].used = 0;
    if (!--s->count)
        s->tail = CPX_NONE;
    return cpx_unpin(s, *out);
}

enum cpx_status cpx_batch(struct cpx_store *s, const struct cpx_record *records,
                          size_t n, size_t *accepted)
{
    if (!s || !accepted || (!records && n))
        return CPX_INVALID;
    *accepted = 0;
    while (*accepted < n)
    {
        struct cpx_handle h;
        enum cpx_status status = cpx_allocate(s, &records[*accepted], &h);
        if (status != CPX_OK)
            return status;
        status = cpx_push(s, h);
        if (status != CPX_OK)
        {
            if (CPX_MUTANT != 2)
                (void)cpx_release(s, h);
            return status;
        }
        ++*accepted;
    }
    return CPX_OK;
}

enum cpx_status cpx_lease_begin(struct cpx_lease *l, struct cpm_arena *a,
                                struct cpm_span span, uint64_t cookie)
{
    if (!l || l->pending || !cookie || cpm_pin(a, span) != CPM_OK)
        return CPX_INVALID;
    *l = (struct cpx_lease){a, cookie, 1, 0};
    return CPX_OK;
}

enum cpx_status cpx_lease_cancel(struct cpx_lease *l)
{
    if (!l || !l->pending)
        return CPX_INVALID;
    l->cancelled = 1;
    return CPX_PENDING;
}

enum cpx_status cpx_lease_complete(struct cpx_lease *l, uint64_t cookie,
                                   int quiescent)
{
    if (!l || !l->pending || l->cookie != cookie)
        return CPX_INVALID;
    if (!quiescent)
        return CPX_PENDING;
    if (cpm_unpin(l->arena) != CPM_OK)
        return CPX_INVALID;
    l->pending = 0;
    return CPX_OK;
}
