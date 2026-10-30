/*
 * Path-algorithm consumer of the immutable CSR snapshot built by indexed.c.
 * It borrows the snapshot by const reference and must finish before the owner
 * is modified or destroyed.
 */
#include "containers.h"

enum
{
    MAX_VERTICES = 8,
    MAX_EDGES = 32
};

/* Re-validates the snapshot instead of trusting the builder: offsets must
 * start at 0, be non-decreasing, end at the edge count, and every edge in row
 * i must start at i and end at a real vertex.  Reading edges only after this
 * passes keeps every later index in range. */
static int csr_valid(const struct cd_csr *g)
{
    if (g->vertices > MAX_VERTICES || g->count > MAX_EDGES ||
        g->offset[0] != 0 || g->offset[g->vertices] != g->count)
    {
        return 0;
    }
    for (unsigned i = 0; i < g->vertices; ++i)
    {
        if (g->offset[i] > g->offset[i + 1] || g->offset[i + 1] > g->count)
        {
            return 0;
        }
    }
    for (unsigned i = 0; i < g->vertices; ++i)
    {
        for (unsigned j = g->offset[i]; j < g->offset[i + 1]; ++j)
        {
            if (g->edges[j].from != i || g->edges[j].to >= g->vertices)
            {
                return 0;
            }
        }
    }
    return 1;
}

/* Breadth-first edge-count distances from `start`; weights are ignored.
 * out[v] is UINT32_MAX for an unreachable vertex (including every v at or
 * above g->vertices).  CD_INVALID, with out untouched, for a null argument, a
 * start outside the graph or a snapshot that fails validation. */
enum cd_status cd_csr_bfs(const struct cd_csr *g, unsigned start,
                          uint32_t out[MAX_VERTICES])
{
    if (!g || !out || start >= g->vertices || !csr_valid(g))
    {
        return CD_INVALID;
    }
    uint32_t distance[MAX_VERTICES];
    for (unsigned i = 0; i < MAX_VERTICES; ++i)
    {
        distance[i] = UINT32_MAX;
    }
    /* Each vertex is queued at most once, so MAX_VERTICES entries suffice. */
    unsigned queue[MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 1;
    distance[start] = 0;
    queue[0] = start;
    while (head < tail)
    {
        unsigned from = queue[head++];
        for (unsigned j = g->offset[from]; j < g->offset[from + 1]; ++j)
        {
            unsigned to = g->edges[j].to;
            if (distance[to] == UINT32_MAX)
            {
                distance[to] = distance[from] + 1;
                queue[tail++] = to;
            }
        }
    }
    for (unsigned i = 0; i < MAX_VERTICES; ++i)
    {
        out[i] = distance[i];
    }
    return CD_OK;
}
