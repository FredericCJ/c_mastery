#include "object_runtime_marking.h"
#ifndef ORC_MUTANT
#define ORC_MUTANT 0
#endif
#define B(i) ((uint16_t)(UINT32_C(1) << (i)))

static int valid(const struct orc_heap *h, struct orc_handle p)
{
    return h && p.slot >= 1 && p.slot <= 16 && p.generation &&
           h->generation[p.slot - 1] == p.generation &&
           (h->graph.live & B(p.slot - 1));
}

static int nullable(const struct orc_heap *h, struct orc_handle p)
{
    return (!p.slot && !p.generation) || valid(h, p);
}

static int room(const struct orc_heap *h)
{
    return h && h->revision && h->revision < ORC_REV_LIMIT;
}

static void shade(struct orc_heap *h, unsigned node)
{
    if (node && h->graph.color[node - 1] == 0)
    {
        h->graph.color[node - 1] = 1;
        h->work[h->used++] = (uint8_t)node;
    }
}

static void restart(struct orc_heap *h)
{
    unsigned i;
    h->used = 0;
    for (i = 0; i < 16; i++)
        h->graph.color[i] = 0;
    for (i = 0; i < 16; i++)
        if (h->graph.roots & B(i))
            shade(h, i + 1);
}

void orcm_init(struct orc_heap *h, enum orc_policy policy)
{
    unsigned i;
    if (!h)
        return;
    *h = (struct orc_heap){0};
    h->revision = 1;
    h->policy = (uint8_t)policy;
    for (i = 0; i < 16; i++)
        h->generation[i] = 1;
}

int orcm_invariant(const struct orc_heap *h)
{
    unsigned i, j;
    uint16_t queued = 0;
    if (!h || !h->revision || h->revision > ORC_REV_LIMIT ||
        h->policy > ORC_RESTART || h->used > 16)
        return 0;
    for (i = 0; i < h->used; i++)
    {
        unsigned node = h->work[i];
        if (!node || node > 16 || (queued & B(node - 1)))
            return 0;
        queued |= B(node - 1);
    }
    for (i = 0; i < 16; i++)
    {
        if (h->graph.live & B(i))
        {
            if (!h->generation[i] || h->generation[i] > 3 ||
                h->graph.color[i] > 2 ||
                !!h->roots[i] != !!(h->graph.roots & B(i)))
                return 0;
            if (h->graph.active &&
                !!(queued & B(i)) != (h->graph.color[i] == 1))
                return 0;
            for (j = 0; j < 2; j++)
            {
                unsigned t = h->graph.edge[i][j];
                if (t > 16 || (t && !(h->graph.live & B(t - 1))))
                    return 0;
                if (h->graph.active && h->graph.color[i] == 2 && t &&
                    h->graph.color[t - 1] == 0)
                    return 0;
            }
        }
        else if (h->roots[i] || h->graph.color[i] || h->graph.edge[i][0] ||
                 h->graph.edge[i][1] || (h->graph.roots & B(i)))
            return 0;
    }
    return h->graph.active || !h->used;
}

enum or_status orcm_allocate(struct orc_heap *h, struct orc_handle first,
                             struct orc_handle second, struct orc_handle *out)
{
    unsigned i;
    struct orc_heap n;
    if (!h || !out || h->policy > ORC_RESTART || !nullable(h, first) ||
        !nullable(h, second))
        return OR_INVALID;
    if (!room(h))
        return OR_FULL;
    for (i = 0; i < 16; i++)
        if (!(h->graph.live & B(i)) && h->generation[i] <= 3)
            break;
    if (i == 16)
        return OR_FULL;
    n = *h;
    n.graph.live |= B(i);
    n.graph.roots |= B(i);
    n.roots[i] = 1;
    n.graph.edge[i][0] = first.slot;
    n.graph.edge[i][1] = second.slot;
    n.graph.color[i] = 0;
    if (n.graph.active)
    {
        if (n.policy == ORC_BLACK)
        {
            n.graph.color[i] = 2;
            if (ORC_MUTANT != 1)
            {
                shade(&n, first.slot);
                shade(&n, second.slot);
            }
        }
        else if (n.policy == ORC_GRAY)
        {
            if (ORC_MUTANT == 2)
                n.graph.color[i] = 2;
            else
                shade(&n, i + 1);
        }
        else
        {
            restart(&n);
            n.restarts++;
        }
    }
    n.revision++;
    *h = n;
    *out = (struct orc_handle){(uint8_t)(i + 1), n.generation[i]};
    return OR_OK;
}

enum or_status orcm_retain(struct orc_heap *h, struct orc_handle p)
{
    enum or_status s;
    int white;
    if (!valid(h, p))
        return OR_INVALID;
    if (!room(h) || h->roots[p.slot - 1] == 255)
        return OR_FULL;
    white = h->graph.active && h->graph.color[p.slot - 1] == 0;
    if (ORC_MUTANT == 4 && white)
        h->graph.roots |= B(p.slot - 1);
    else
    {
        s = or_graph_root(&h->graph, p.slot, 1);
        if (s != OR_OK)
            return s;
        if (white)
            h->work[h->used++] = p.slot;
    }
    h->roots[p.slot - 1]++;
    h->revision++;
    return OR_OK;
}

enum or_status orcm_release(struct orc_heap *h, struct orc_handle p)
{
    enum or_status s;
    if (!valid(h, p))
        return OR_INVALID;
    if (!h->roots[p.slot - 1])
        return OR_STATE;
    if (!room(h))
        return OR_FULL;
    if (h->roots[p.slot - 1] == 1)
    {
        s = or_graph_root(&h->graph, p.slot, 0);
        if (s != OR_OK)
            return s;
    }
    h->roots[p.slot - 1]--;
    h->revision++;
    return OR_OK;
}

enum or_status orcm_store(struct orc_heap *h, struct orc_handle owner,
                          unsigned field, struct orc_handle target)
{
    enum or_status s;
    int white;
    if (!valid(h, owner) || field >= 2 || !nullable(h, target))
        return OR_INVALID;
    if (!room(h))
        return OR_FULL;
    white = h->graph.active && h->graph.color[owner.slot - 1] == 2 &&
            target.slot && h->graph.color[target.slot - 1] == 0;
    s = or_graph_store(&h->graph, owner.slot, field, target.slot);
    if (s != OR_OK)
        return s;
    if (white)
        h->work[h->used++] = target.slot;
    h->revision++;
    return OR_OK;
}

enum or_status orcm_read(const struct orc_heap *h, struct orc_handle owner,
                         struct orc_handle edges[2])
{
    unsigned j;
    struct orc_handle result[2];
    if (!valid(h, owner) || !edges)
        return OR_INVALID;
    for (j = 0; j < 2; j++)
    {
        unsigned target = h->graph.edge[owner.slot - 1][j];
        if (target > 16 || (target && !(h->graph.live & B(target - 1))))
            return OR_STATE;
        result[j] = (struct orc_handle){(uint8_t)target,
                                        target ? h->generation[target - 1] : 0};
    }
    edges[0] = result[0];
    edges[1] = result[1];
    return OR_OK;
}

enum or_status orcm_begin(struct orc_heap *h)
{
    enum or_status s;
    unsigned i;
    if (!h)
        return OR_INVALID;
    if (h->graph.active)
        return OR_STATE;
    if (!room(h))
        return OR_FULL;
    s = or_mark_begin(&h->graph);
    if (s != OR_OK)
        return s;
    h->used = 0;
    for (i = 0; i < 16; i++)
        if (h->graph.color[i] == 1)
            h->work[h->used++] = (uint8_t)(i + 1);
    h->cycles++;
    h->revision++;
    return OR_OK;
}

enum or_status orcm_step(struct orc_heap *h, unsigned budget, unsigned *scanned)
{
    struct orc_heap n;
    unsigned count = 0, j;
    if (!h || !scanned)
        return OR_INVALID;
    if (!h->graph.active)
        return OR_STATE;
    if (!h->used || !budget)
    {
        *scanned = 0;
        return h->used ? OR_OK : OR_DONE;
    }
    if (!room(h))
        return OR_FULL;
    n = *h;
    while (n.used && count < budget)
    {
        unsigned node = n.work[--n.used];
        for (j = 0; j < 2; j++)
            shade(&n, n.graph.edge[node - 1][j]);
        n.graph.color[node - 1] = 2;
        count++;
    }
    n.scans += count;
    n.revision++;
    *h = n;
    *scanned = count;
    return n.used ? OR_OK : OR_DONE;
}

enum or_status orcm_prepare(const struct orc_heap *h, struct orc_finish *out)
{
    unsigned i;
    uint16_t white = 0;
    if (!h || !out)
        return OR_INVALID;
    if (!h->graph.active || h->used)
        return OR_STATE;
    for (i = 0; i < 16; i++)
        if ((h->graph.live & B(i)) && h->graph.color[i] == 0)
            white |= B(i);
    *out = (struct orc_finish){h, h->revision, white};
    return OR_OK;
}

enum or_status orcm_commit(struct orc_heap *h, struct orc_finish token,
                           uint16_t *dead)
{
    unsigned i;
    if (!h || !dead || token.owner != h)
        return OR_INVALID;
    if (!h->graph.active)
        return OR_STATE;
    if (ORC_MUTANT != 3 && (token.revision != h->revision || h->used))
        return OR_STATE;
    if (!room(h))
        return OR_FULL;
    for (i = 0; i < 16; i++)
        if (token.white & B(i))
        {
            h->graph.live &= (uint16_t)~B(i);
            h->graph.roots &= (uint16_t)~B(i);
            h->graph.edge[i][0] = h->graph.edge[i][1] = 0;
            h->graph.color[i] = 0;
            h->roots[i] = 0;
            h->generation[i]++;
        }
    h->used = 0;
    h->graph.active = 0;
    h->revision++;
    *dead = token.white;
    return OR_OK;
}
