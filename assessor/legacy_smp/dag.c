#include "dag.h"
#ifndef DG_MUTANT
#define DG_MUTANT 0
#endif
static unsigned bits(uint16_t value)
{
    unsigned n = 0;
    while (value)
    {
        n += value & 1u;
        value >>= 1;
    }
    return n;
}

static int dg_lock(struct sa_dg *g, struct sl_guard *guard)
{
    *guard = (struct sl_guard){0};
    return sl_enter(&g->lock, NULL, guard, NULL) == SL_OK;
}

static int sum(uint64_t a, uint64_t b, uint64_t *out)
{
    if (a > UINT64_MAX - b && DG_MUTANT != 8)
        return 0;
    *out = a + b;
    return 1;
}

int dg_init(struct sa_dg *g, uint64_t world, const struct dg_spec *spec)
{
    if (!g || !world || !spec || spec->count > DG_LIMIT ||
        spec->edge_count > DG_EDGES)
        return DG_INVALID;
    struct dg_node next[DG_LIMIT] = {0};
    uint64_t work = 0, span = 0;
    for (unsigned i = 0; i < spec->count; i++)
    {
        next[i].payload = next[i].input = spec->payloads[i];
        next[i].cost = spec->costs[i];
        next[i].owner = 4;
        if (!sum(work, spec->costs[i], &work))
            return DG_RANGE;
    }
    for (unsigned i = 0; i < spec->edge_count; i++)
    {
        unsigned from = spec->edges[i].from, to = spec->edges[i].to;
        if (from >= spec->count || to >= spec->count)
            return DG_INVALID;
        uint16_t bit = (uint16_t)(1u << to);
        if ((next[from].successors & bit) && DG_MUTANT != 1)
            return DG_DUPLICATE;
        next[from].successors |= bit;
        next[to].predecessors |= (uint16_t)(1u << from);
    }
    unsigned degree[DG_LIMIT] = {0}, order[DG_LIMIT], count = 0;
    uint16_t visited = 0;
    for (unsigned i = 0; i < spec->count; i++)
    {
        degree[i] = bits(next[i].predecessors);
        next[i].remaining = degree[i];
        next[i].state = degree[i] ? DG_WAITING : DG_READY;
        next[i].ready_events = degree[i] ? 0 : 1;
    }
    while (count < spec->count)
    {
        unsigned selected = spec->count;
        for (unsigned i = 0; i < spec->count; i++)
            if (!(visited & (1u << i)) && !degree[i])
            {
                selected = i;
                break;
            }
        if (selected == spec->count)
        {
            if (DG_MUTANT == 2)
                break;
            return DG_CYCLE;
        }
        visited |= (uint16_t)(1u << selected);
        order[count++] = selected;
        for (unsigned j = 0; j < spec->count; j++)
            if (next[selected].successors & (1u << j))
                degree[j]--;
    }
    for (unsigned k = 0; k < count; k++)
    {
        unsigned i = order[k];
        uint64_t longest = 0;
        for (unsigned p = 0; p < spec->count; p++)
            if ((next[i].predecessors & (1u << p)) && next[p].span > longest)
                longest = next[p].span;
        if (!sum(longest, next[i].cost, &next[i].span))
            return DG_RANGE;
        if (next[i].span > span)
            span = next[i].span;
    }
    g->world = world;
    g->work = work;
    g->span = span;
    g->count = spec->count;
    g->edges = spec->edge_count;
    g->sealed = g->terminal = 0;
    (void)sl_init(&g->lock, SL_TAS, 1, 0);
    for (unsigned i = 0; i < DG_LIMIT; i++)
        g->nodes[i] = next[i];
    return DG_OK;
}

/* Caller holds graph lock. Each newly terminal node is enqueued once; at most
 * the16 graph nodes are ever queued, including transitive failure propagation. */
static void terminal(struct sa_dg *g, unsigned node, unsigned state,
                     uint64_t result, unsigned error)
{
    unsigned pending[DG_LIMIT], begin = 0, end = 0;
    struct dg_node *first = &g->nodes[node];
    first->state = state;
    first->result = state == DG_DONE && DG_MUTANT != 9 ? result : 0;
    first->error = error;
    g->terminal++;
    pending[end++] = node;
    while (begin < end)
    {
        unsigned from = pending[begin++];
        struct dg_node *source = &g->nodes[from];
        for (unsigned i = 0; i < g->count; i++)
            if (source->successors & (1u << i))
            {
                struct dg_node *target = &g->nodes[i];
                if (target->state != DG_WAITING)
                    continue;
                unsigned failed = source->state != DG_DONE && DG_MUTANT != 5;
                if (!failed &&
                    !sum(target->input, source->result, &target->input))
                    failed = 2;
                if (failed)
                {
                    target->state = failed == 2 ? DG_FAILED : DG_SKIPPED;
                    target->error = failed == 2 ? DG_RANGE : source->error;
                    target->result = 0;
                    g->terminal++;
                    pending[end++] = i;
                    continue;
                }
                target->remaining--;
                if (DG_MUTANT == 3)
                    target->remaining = 0;
                if (!target->remaining)
                {
                    target->state = DG_READY;
                    target->ready_events++;
                }
            }
    }
    if (DG_MUTANT == 9 && state == DG_DONE)
        first->result = result;
}

int dg_append(struct sa_dg *g, uint64_t payload, uint64_t cost,
              const unsigned *parents, unsigned count, unsigned *out)
{
    if (!g || !out || (count && !parents) || count > DG_LIMIT)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    int status = DG_OK;
    uint16_t mask = 0;
    uint64_t work = 0, span = 0, input = payload;
    unsigned remaining = 0, skip = 0;
    if (g->sealed)
        status = DG_CLOSED;
    else if (g->count == DG_LIMIT || count > DG_EDGES - g->edges)
        status = DG_CAPACITY;
    else
    {
        if (DG_MUTANT == 6)
            *out = g->count;
        for (unsigned i = 0; i < count; i++)
        {
            unsigned p = parents[i];
            if (p >= g->count)
            {
                status = DG_INVALID;
                break;
            }
            if (mask & (1u << p))
            {
                status = DG_DUPLICATE;
                break;
            }
            mask |= (uint16_t)(1u << p);
            struct dg_node *n = &g->nodes[p];
            if (n->span > span)
                span = n->span;
            if (n->state == DG_DONE)
            {
                if (!sum(input, n->result, &input))
                {
                    status = DG_RANGE;
                    break;
                }
            }
            else if (n->state == DG_FAILED || n->state == DG_SKIPPED ||
                     n->state == DG_CANCELLED)
                skip = 1;
            else
                remaining++;
        }
        if (status == DG_OK &&
            (!sum(g->work, cost, &work) || !sum(span, cost, &span)))
            status = DG_RANGE;
    }
    if (status == DG_OK)
    {
        unsigned index = g->count;
        struct dg_node node = {0};
        node.payload = payload;
        node.input = input;
        node.cost = cost;
        node.span = span;
        node.predecessors = mask;
        node.remaining = remaining;
        node.owner = 4;
        node.state = skip ? DG_SKIPPED : remaining ? DG_WAITING : DG_READY;
        node.ready_events = node.state == DG_READY;
        node.error = skip ? DG_FAILED : 0;
        g->nodes[index] = node;
        g->count++;
        g->edges += count;
        g->work = work;
        if (span > g->span)
            g->span = span;
        if (skip)
            g->terminal++;
        for (unsigned i = 0; i < index; i++)
            if (mask & (1u << i))
                g->nodes[i].successors |= (uint16_t)(1u << index);
        *out = index;
    }
    (void)sl_leave(&guard, NULL);
    return status;
}

int dg_seal(struct sa_dg *g)
{
    if (!g)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    g->sealed = 1;
    (void)sl_leave(&guard, NULL);
    return DG_OK;
}

int dg_claim(struct sa_dg *g, unsigned worker, struct dg_claim *out)
{
    if (!g || !out || worker >= 4)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    int status = DG_EMPTY;
    for (unsigned i = 0; i < g->count; i++)
        if (g->nodes[i].state == DG_READY)
        {
            struct dg_node *n = &g->nodes[i];
            n->state = DG_RUNNING;
            n->owner = worker;
            *out = (struct dg_claim){g->world, n->input, n->cost, i, worker};
            status = DG_OK;
            break;
        }
    (void)sl_leave(&guard, NULL);
    return status;
}

int dg_finish(struct sa_dg *g, struct dg_claim claim, unsigned outcome,
              uint64_t result)
{
    if (!g || claim.worker >= 4 || (outcome != DG_DONE && outcome != DG_FAILED))
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    int status = DG_STALE;
    if (claim.world == g->world && claim.node < g->count)
    {
        struct dg_node *n = &g->nodes[claim.node];
        if (n->owner == claim.worker &&
            (n->state == DG_RUNNING || (DG_MUTANT == 4 && n->state == DG_DONE)))
        {
            unsigned state = n->cancel ? DG_CANCELLED : outcome;
            terminal(g, claim.node, state, result,
                     state == DG_DONE ? 0 : state);
            status = DG_OK;
        }
    }
    (void)sl_leave(&guard, NULL);
    return status;
}

int dg_cancel(struct sa_dg *g, unsigned node)
{
    if (!g)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    int status = DG_INVALID;
    if (node < g->count)
    {
        struct dg_node *n = &g->nodes[node];
        if (n->state == DG_RUNNING)
        {
            n->cancel = 1;
            if (DG_MUTANT == 7)
                terminal(g, node, DG_CANCELLED, 0, DG_CANCELLED);
            status = DG_REQUESTED;
        }
        else if (n->state == DG_WAITING || n->state == DG_READY)
        {
            terminal(g, node, DG_CANCELLED, 0, DG_CANCELLED);
            status = DG_OK;
        }
        else
            status = DG_STALE;
    }
    (void)sl_leave(&guard, NULL);
    return status;
}

int dg_snapshot(struct sa_dg *g, struct dg_snapshot *out)
{
    if (!g || !out)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    struct dg_snapshot next = {0};
    next.world = g->world;
    next.work = g->work;
    next.span = g->span;
    next.count = g->count;
    next.edges = g->edges;
    next.sealed = g->sealed;
    next.terminal = g->terminal;
    for (unsigned i = 0; i < g->count; i++)
    {
        struct dg_node *n = &g->nodes[i];
        next.states[i] = n->state;
        next.ready_events[i] = n->ready_events;
        next.results[i] = n->result;
        next.ready += n->state == DG_READY;
        next.running += n->state == DG_RUNNING;
    }
    *out = next;
    (void)sl_leave(&guard, NULL);
    return DG_OK;
}

int dg_drained(struct sa_dg *g)
{
    if (!g)
        return DG_INVALID;
    struct sl_guard guard;
    if (!dg_lock(g, &guard))
        return DG_BUSY;
    int status = g->sealed && g->terminal == g->count ? DG_OK : DG_BUSY;
    (void)sl_leave(&guard, NULL);
    return status;
}
