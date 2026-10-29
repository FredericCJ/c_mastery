#include "event_follow.h"
#include <limits.h>
#ifndef PE_MUTANT
#define PE_MUTANT 0
#endif
int pf_post(struct pf_hsm *h, unsigned event)
{
    if (!h || event > PF_LOCAL || h->head > 3 || h->count > 4)
    {
        return PT_INVALID;
    }
    if (h->count == 4)
    {
        return PT_RANGE;
    }
    h->events[(h->head + h->count) % 4] = event;
    ++h->count;
    return PT_OK;
}

int pf_dispatch(struct pf_hsm *h, unsigned event, unsigned guard,
                unsigned local, pf_entry_fn entry, void *ctx,
                unsigned char *trace, size_t cap, size_t *used)
{
    if (!h || !used || (!trace && cap) || event > PF_LOCAL || guard > 1 ||
        local > 1)
    {
        return PT_INVALID;
    }
    if (h->in_step)
    {
        return PT_STATE;
    }
    struct pt_machine copy = h->machine;
    unsigned char bytes[6];
    size_t n = 0;
    int r;
    if (event == PF_GUARDED)
    {
        if (copy.active != PT_IDLE && copy.active != PT_BUSY)
        {
            return PT_STATE;
        }
        if (guard)
        {
            event = PT_PING;
        }
        else
        {
            if (PE_MUTANT == 1)
            {
                return PT_STATE;
            }
            event = PT_FAIL;
        }
    }
    if (event == PF_LOCAL)
    {
        if (copy.active != PT_IDLE && copy.active != PT_BUSY)
        {
            return PT_STATE;
        }
        bytes[n++] = (unsigned char)(0x80u | copy.active);
        if (!local || PE_MUTANT == 4)
        {
            bytes[n++] = 0x80u | PT_ONLINE;
            bytes[n++] = PT_ONLINE;
        }
        bytes[n++] = PT_IDLE;
        copy.active = PT_IDLE;
        r = PT_OK;
    }
    else
    {
        r = pt_dispatch(&copy, event, bytes, sizeof bytes, &n);
    }
    if (r)
    {
        return r;
    }
    if (n > cap)
    {
        return PT_RANGE;
    }
    h->in_step = 1;
    h->machine = copy;
    for (size_t i = 0; i < n; ++i)
    {
        trace[i] = bytes[i];
    }
    *used = n;
    for (size_t i = 0; i < n; ++i)
    {
        if (!(bytes[i] & 0x80u) && entry && entry(ctx, bytes[i]))
        {
            h->in_step = 0;
            return PT_IO;
        }
    }
    h->in_step = 0;
    return PT_OK;
}

static int valid(struct pf_active *a)
{
    return a && a->lock.enter && a->lock.leave;
}

int pf_active_post(struct pf_active *a, unsigned event)
{
    if (!valid(a))
    {
        return PT_INVALID;
    }
    a->lock.enter(a->lock.context);
    int r = pf_post(&a->hsm, event);
    a->lock.leave(a->lock.context);
    return r;
}

int pf_active_step(struct pf_active *a, unsigned char *trace, size_t cap,
                   size_t *used)
{
    if (!valid(a) || !used)
    {
        return PT_INVALID;
    }
    a->lock.enter(a->lock.context);
    int r = PT_PENDING;
    if (a->hsm.count)
    {
        unsigned event = a->hsm.events[a->hsm.head];
        r = pf_dispatch(&a->hsm, event, 0, 1, 0, 0, trace, cap, used);
        if (!r || PE_MUTANT == 2)
        {
            a->hsm.head = (a->hsm.head + 1) % 4;
            --a->hsm.count;
        }
    }
    a->lock.leave(a->lock.context);
    return r;
}

int pf_active_discard(struct pf_active *a, unsigned *event)
{
    if (!valid(a) || !event)
    {
        return PT_INVALID;
    }
    a->lock.enter(a->lock.context);
    int r = PT_PENDING;
    if (a->hsm.count)
    {
        if (a->discarded == UINT32_MAX)
        {
            r = PT_RANGE;
        }
        else
        {
            *event = a->hsm.events[a->hsm.head];
            a->hsm.head = (a->hsm.head + 1) % 4;
            --a->hsm.count;
            if (PE_MUTANT != 5)
            {
                ++a->discarded;
            }
            r = PT_OK;
        }
    }
    a->lock.leave(a->lock.context);
    return r;
}

static int expand(struct pf_reactive *b, const struct pf_node *nodes, size_t n,
                  unsigned node, unsigned *index)
{
    if (node >= n || b->count == 16 || nodes[node].kind > PT_BT_SELECTOR)
    {
        return 0;
    }
    unsigned own = b->count++;
    *index = own;
    b->frames[own] = (struct pf_occurrence){.node = node};
    if (nodes[node].kind != PT_BT_LEAF)
    {
        if (nodes[node].left <= node || nodes[node].right <= node)
        {
            return 0;
        }
        if (!expand(b, nodes, n, nodes[node].left, &b->frames[own].left) ||
            !expand(b, nodes, n, nodes[node].right, &b->frames[own].right))
        {
            return 0;
        }
    }
    return 1;
}

int pf_reactive_init(struct pf_reactive *b, const struct pf_node *nodes,
                     size_t n)
{
    if (!b || !nodes || !n || n > 8)
    {
        return PT_INVALID;
    }
    struct pf_reactive copy = {0};
    unsigned root;
    if (!expand(&copy, nodes, n, 0, &root))
    {
        return PT_RANGE;
    }
    *b = copy;
    return PT_OK;
}

static void cancel(struct pf_reactive *b, pf_abort_fn abort, void *ctx)
{
    unsigned mask = b->active;
    b->active = 0;
    for (unsigned i = 0; i < b->count; ++i)
    {
        b->frames[i].cursor = 0;
        if ((mask & (1u << i)) && PE_MUTANT != 3)
        {
            abort(ctx, i, b->frames[i].node);
        }
    }
}

int pf_reactive_cancel(struct pf_reactive *b, pf_abort_fn abort, void *ctx)
{
    if (!b || !abort || !b->count || b->count > 16)
    {
        return PT_INVALID;
    }
    if (b->ticking)
    {
        return PT_STATE;
    }
    b->ticking = 1;
    cancel(b, abort, ctx);
    b->ticking = 0;
    return PT_OK;
}

static unsigned tick(struct pf_reactive *b, const struct pf_node *nodes,
                     unsigned i, pf_leaf_fn leaf, void *ctx)
{
    struct pf_occurrence *f = &b->frames[i];
    unsigned kind = nodes[f->node].kind;
    if (kind == PT_BT_LEAF)
    {
        unsigned slot = PE_MUTANT == 6 ? f->node : i;
        b->active |= 1u << slot;
        unsigned r = leaf(ctx, i, f->node);
        if (r <= PT_BT_FAILURE)
        {
            b->active &= ~(1u << slot);
        }
        return r;
    }
    while (f->cursor < 2)
    {
        unsigned r = tick(b, nodes, f->cursor ? f->right : f->left, leaf, ctx);
        if (r >= PT_BT_RUNNING)
        {
            return r;
        }
        if ((kind == PT_BT_SEQUENCE && r == PT_BT_FAILURE) ||
            (kind == PT_BT_SELECTOR && r == PT_BT_SUCCESS))
        {
            f->cursor = 0;
            return r;
        }
        ++f->cursor;
    }
    f->cursor = 0;
    return kind == PT_BT_SEQUENCE ? PT_BT_SUCCESS : PT_BT_FAILURE;
}

int pf_reactive_tick(struct pf_reactive *b, const struct pf_node *nodes,
                     size_t n, unsigned guard, pf_leaf_fn leaf,
                     pf_abort_fn abort, void *ctx, unsigned *out)
{
    if (!b || !nodes || !leaf || !abort || !out || !n || n > 8 || guard > 1 ||
        !b->count || b->count > 16)
    {
        return PT_INVALID;
    }
    if (b->ticking)
    {
        return PT_STATE;
    }
    /* Immutable topology is a lifetime precondition; check frame bounds defensively. */
    for (unsigned i = 0; i < b->count; ++i)
    {
        if (b->frames[i].node >= n || b->frames[i].cursor > 1)
        {
            return PT_INVALID;
        }
    }
    b->ticking = 1;
    if (!guard)
    {
        cancel(b, abort, ctx);
        b->ticking = 0;
        *out = PT_BT_FAILURE;
        return PT_OK;
    }
    unsigned r = tick(b, nodes, 0, leaf, ctx);
    b->ticking = 0;
    if (r > PT_BT_RUNNING)
    {
        return PT_INVALID;
    }
    *out = r;
    return PT_OK;
}
