#include "algorithms.h"
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static int graph_valid(const struct cd_csr *g)
{
    if (!g || g->vertices > 8 || g->count > 32 || g->offset[0] ||
        g->offset[g->vertices] != g->count)
        return 0;
    for (unsigned v = 0; v < g->vertices; ++v)
    {
        if (g->offset[v] > g->offset[v + 1] || g->offset[v + 1] > g->count)
            return 0;
        for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
            if (g->edges[e].from != v || g->edges[e].to >= g->vertices)
                return 0;
    }
    return 1;
}

static struct ad_traversal empty(void)
{
    struct ad_traversal t = {0};
    for (unsigned i = 0; i < 8; ++i)
    {
        t.distance[i] = UINT32_MAX;
        t.root[i] = 8;
    }
    return t;
}

enum ad_status ad_reachable_stack(const struct cd_csr *g, unsigned start,
                                  unsigned *stack, size_t capacity,
                                  unsigned *out)
{
    if (!graph_valid(g) || !out || start >= g->vertices)
        return AD_INVALID;
    if (capacity < g->vertices || !stack)
        return AD_SPACE;
    unsigned seen = 1u << start, used = 1;
    stack[0] = start;
    while (used)
    {
        unsigned v = stack[--used];
        for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
        {
            unsigned to = g->edges[e].to;
            if (!(seen & (1u << to)))
            {
                seen |= 1u << to;
                stack[used++] = to;
            }
        }
    }
    *out = seen;
    return AD_OK;
}

enum ad_status ad_bfs(const struct cd_csr *g, unsigned start, unsigned forest,
                      unsigned *queue, size_t capacity,
                      struct ad_traversal *out)
{
    if (!graph_valid(g) || !out || forest > 1 ||
        (g->vertices && start >= g->vertices))
        return AD_INVALID;
    if (capacity < g->vertices || (g->vertices && !queue))
        return AD_SPACE;
    struct ad_traversal t = empty();
    unsigned seen = 0;
    for (unsigned pass = 0; pass < (forest ? g->vertices : 1u); ++pass)
    {
        if (!g->vertices)
            break;
        unsigned root = pass ? ((pass <= start) ? pass - 1 : pass) : start;
        if (seen & (1u << root))
            continue;
        unsigned head = 0, tail = 1;
        queue[0] = root;
        seen |= 1u << root;
        t.distance[root] = 0;
        t.root[root] = root;
        ++t.roots;
        while (head < tail)
        {
            if (t.peak < tail - head)
                t.peak = tail - head;
            unsigned v = queue[head++];
            t.order[t.count++] = v;
            for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
            {
                ++t.edges;
                unsigned w = g->edges[e].to;
                if (!(seen & (1u << w)))
                {
                    seen |= 1u << w;
                    queue[tail++] = w;
                    t.distance[w] = t.distance[v] + 1;
                    t.root[w] = root;
                }
            }
        }
    }
    *out = t;
    return AD_OK;
}

enum ad_status ad_dfs(const struct cd_csr *g, unsigned start, unsigned forest,
                      struct ad_frame *frames, size_t capacity,
                      struct ad_traversal *out)
{
    if (!graph_valid(g) || !out || forest > 1 ||
        (g->vertices && start >= g->vertices))
        return AD_INVALID;
    /* Conservative admission gives unchanged output before any frontier write. */
    if (capacity < g->vertices || (g->vertices && !frames))
        return AD_SPACE;
    struct ad_traversal t = empty();
    unsigned seen = 0;
    for (unsigned pass = 0; pass < (forest ? g->vertices : 1u); ++pass)
    {
        if (!g->vertices)
            break;
        unsigned root = pass ? ((pass <= start) ? pass - 1 : pass) : start;
        if (seen & (1u << root))
            continue;
        unsigned used = 1;
        frames[0] = (struct ad_frame){root, g->offset[root]};
        seen |= 1u << root;
        t.order[t.count++] = root;
        t.root[root] = root;
        t.distance[root] = 0;
        ++t.roots;
        while (used)
        {
            if (t.peak < used)
                t.peak = used;
            struct ad_frame *f = &frames[used - 1];
            if (f->next == g->offset[f->vertex + 1])
            {
                t.finish[t.finished++] = f->vertex;
                --used;
                continue;
            }
            unsigned v = f->vertex, w = g->edges[f->next++].to;
            ++t.edges;
            if (!(seen & (1u << w)))
            {
                seen |= 1u << w;
                t.order[t.count++] = w;
                t.root[w] = root;
                t.distance[w] = t.distance[v] + 1;
                frames[used++] = (struct ad_frame){w, g->offset[w]};
            }
        }
    }
    *out = t;
    return AD_OK;
}

enum ad_status ad_topological(const struct cd_csr *g, unsigned out[8])
{
    if (!graph_valid(g) || !out)
    {
        return AD_INVALID;
    }
    unsigned degree[8] = {0}, order[8] = {0}, done = 0;
    for (unsigned e = 0; e < g->count; ++e)
        ++degree[g->edges[e].to];
    for (unsigned at = 0; at < g->vertices; ++at)
    {
        unsigned v = 0;
        while (v < g->vertices && ((done & (1u << v)) || degree[v]))
            ++v;
        if (v == g->vertices)
            return AD_FAILED;
        done |= 1u << v;
        order[at] = v;
        for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
            --degree[g->edges[e].to];
    }
    for (unsigned i = 0; i < 8; ++i)
    {
        out[i] = order[i];
    }
    return AD_OK;
}

enum ad_status ad_dijkstra(const struct cd_csr *g, unsigned start,
                           uint64_t limit, uint64_t out[8],
                           unsigned previous[8])
{
    if (!graph_valid(g) || !out || !previous || start >= g->vertices ||
        limit == UINT64_MAX)
        return AD_INVALID;
    for (unsigned e = 0; e < g->count; ++e)
        if (g->edges[e].weight < 0)
            return AD_INVALID;
    uint64_t distance[8];
    unsigned pred[8], done = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        distance[i] = UINT64_MAX;
        pred[i] = 8;
    }
    distance[start] = 0;
    pred[start] = start;
    for (unsigned step = 0; step < g->vertices; ++step)
    {
        unsigned v = 8;
        for (unsigned i = 0; i < g->vertices; ++i)
            if (!(done & (1u << i)) && distance[i] != UINT64_MAX &&
                (v == 8 || distance[i] < distance[v]))
                v = i;
        if (v == 8)
            break;
        done |= 1u << v;
        for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
        {
            unsigned w = g->edges[e].to;
            uint64_t weight = (unsigned)g->edges[e].weight;
            if (weight > limit - distance[v])
                return AD_OVERFLOW;
            uint64_t next = distance[v] + weight;
            if (!(done & (1u << w)) && next < distance[w])
            {
                distance[w] = next;
                pred[w] = v;
            }
        }
    }
    for (unsigned i = 0; i < 8; ++i)
    {
        out[i] = distance[i];
        previous[i] = pred[i];
    }
    return AD_OK;
}

enum ad_status ad_intervals(const struct ad_interval *p, size_t n,
                            unsigned out[8], size_t *count)
{
    if (n > 8 || (n && !p) || !out || !count)
    {
        return AD_INVALID;
    }
    unsigned indices[8], selected[8];
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i].start >= p[i].end)
            return AD_INVALID;
        indices[i] = (unsigned)i;
    }
    for (size_t i = 1; i < n; ++i)
    {
        unsigned saved = indices[i];
        size_t j = i;
        while (j && p[indices[j - 1]].end > p[saved].end)
        {
            indices[j] = indices[j - 1];
            --j;
        }
        indices[j] = saved;
    }
    size_t used = 0;
    int end = 0;
    for (size_t i = 0; i < n; ++i)
    {
        const struct ad_interval *x = &p[indices[i]];
        if (!used || x->start >= end)
        {
            selected[used++] = x->label;
            end = x->end;
        }
    }
    for (size_t i = 0; i < used; ++i)
    {
        out[i] = selected[i];
    }
    *count = used;
    return AD_OK;
}

enum ad_status ad_knapsack(const struct ad_item *p, size_t n, unsigned capacity,
                           unsigned exact, unsigned unbounded, uint32_t *out)
{
    if (n > 8 || (n && !p) || capacity > 32 || exact > 1 || unbounded > 1 ||
        !out)
    {
        return AD_INVALID;
    }
    for (size_t i = 0; i < n; ++i)
        if (!p[i].weight)
            return AD_INVALID;
    uint32_t values[33] = {0};
    unsigned reachable[33];
    for (unsigned c = 0; c <= capacity; ++c)
        reachable[c] = !exact || c == 0;
    for (size_t i = 0; i < n; ++i)
    {
        unsigned weight = p[i].weight;
        if (weight > capacity)
            continue;
        for (unsigned step = 0; step <= capacity - weight; ++step)
        {
            unsigned c =
                (unbounded || AD_MUTANT == 3) ? weight + step : capacity - step;
            if (reachable[c - weight])
            {
                if (p[i].value > UINT32_MAX - values[c - weight])
                    return AD_OVERFLOW;
                uint32_t candidate = values[c - weight] + p[i].value;
                if (!reachable[c] || candidate > values[c])
                {
                    values[c] = candidate;
                    reachable[c] = 1;
                }
            }
        }
    }
    if (!reachable[capacity])
    {
        return AD_MISSING;
    }
    *out = values[capacity];
    return AD_OK;
}
