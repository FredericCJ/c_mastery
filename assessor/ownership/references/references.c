#include "references.h"
#ifndef ORC_MUTATION
#define ORC_MUTATION 0
#endif
struct ow_node;

struct ow_edge
{
    struct ow_node *node;
    unsigned weak;
};

struct ow_world
{
    struct orc_memory memory;
    struct v4_allocator cells;
    struct ow_node *node[2];
    struct ow_edge pending[2];
    unsigned pending_count;
    struct orc_cycle_report report;
};

struct ow_node
{
    struct ow_shared refs;
    struct ow_world *world;
    int32_t *payload;
    struct ow_edge edge;
    unsigned index;
};

static void destroy(void *p)
{
    struct ow_node *n = p;
    struct ow_world *w = n->world;
    int32_t *payload = n->payload;
    struct ow_edge edge = n->edge;
    n->payload = NULL;
    n->edge = (struct ow_edge){0};
    ++w->report.destructions;
    w->cells.release(w->cells.context, payload);
    if (edge.node)
    {
        w->pending[w->pending_count++] = edge;
        ++w->report.queued_releases;
    }
}

static void reclaim(void *p)
{
    struct ow_node *n = p;
    struct ow_world *w = n->world;
    struct orc_memory memory = w->memory;
    w->node[n->index] = NULL;
    ++w->report.reclamations;
    memory.release(memory.context, n);
}

static enum ow_status create(struct ow_world *w, unsigned i)
{
    struct ow_node *n = w->memory.take(w->memory.context, sizeof(*n));
    if (!n)
    {
        return OW_NOMEM;
    }
    int32_t *payload = w->cells.take(w->cells.context, 1);
    if (!payload)
    {
        w->memory.release(w->memory.context, n);
        return OW_NOMEM;
    }
    *payload = (int32_t)(11 + i);
    *n = (struct ow_node){0};
    n->world = w;
    n->payload = payload;
    n->index = i;
    n->refs = (struct ow_shared){1, 1, destroy, reclaim, n};
    w->node[i] = n;
    return OW_OK;
}

static void drain(struct ow_world *w)
{
    while (w->pending_count)
    {
        struct ow_edge e = w->pending[--w->pending_count];
        if (e.weak)
        {
            (void)ow_shared_weak_drop(&e.node->refs);
        }
        else
        {
            (void)ow_shared_drop(&e.node->refs);
        }
    }
}

enum ow_status orc_cycle(struct orc_memory m, struct v4_allocator a, int weak,
                         struct orc_cycle_report *out)
{
    if (!out || !m.take || !m.release || !a.take || !a.release ||
        (weak != 0 && weak != 1))
    {
        return OW_INVALID;
    }
    struct ow_world w = {0};
    w.memory = m;
    w.cells = a;
    for (unsigned i = 0; i < 2; ++i)
    {
        enum ow_status s = create(&w, i);
        if (s != OW_OK)
        {
            for (unsigned j = 0; j < i; ++j)
            {
                (void)ow_shared_drop(&w.node[j]->refs);
            }
            return s;
        }
    }
    struct ow_node *first = w.node[0], *second = w.node[1];
    /* Each explicit observer weak retains its control until the final audit. */
    (void)ow_shared_downgrade(&first->refs);
    (void)ow_shared_downgrade(&second->refs);
    (void)ow_shared_retain(&second->refs);
    first->edge = (struct ow_edge){second, 0};
    if (weak && ORC_MUTATION != 1)
    {
        (void)ow_shared_downgrade(&first->refs);
        second->edge = (struct ow_edge){first, 1};
    }
    else
    {
        (void)ow_shared_retain(&first->refs);
        second->edge = (struct ow_edge){first, 0};
    }
    (void)ow_shared_drop(&first->refs);
    (void)ow_shared_drop(&second->refs);
    drain(&w);
    w.report.retained_values = 2 - w.report.destructions;
    w.report.retained_controls = 2 - w.report.reclamations;
    if (w.report.retained_values)
    {
        /* Exclusive graph maintenance consumes the existing first->second strong edge. */
        (void)ow_shared_downgrade(&second->refs);
        first->edge.weak = 1;
        ++w.report.repair_edges;
        (void)ow_shared_drop(&second->refs);
        drain(&w);
    }
    /* Nodes remain real live control objects because of the two observer weak refs. */
    (void)ow_shared_weak_drop(&first->refs);
    (void)ow_shared_weak_drop(&second->refs);
    *out = w.report;
    return OW_OK;
}
