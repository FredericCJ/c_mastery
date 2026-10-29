#include "containers.h"

enum cd_status cd_csr_bfs(const struct cd_csr *g, unsigned start,
                          uint32_t out[8])
{
    if (!g || !out || g->vertices > 8 || start >= g->vertices ||
        g->count > 32 || g->offset[0] != 0 ||
        g->offset[g->vertices] != g->count)
    {
        return CD_INVALID;
    }
    for (unsigned i = 0; i < g->vertices; ++i)
    {
        if (g->offset[i] > g->offset[i + 1] || g->offset[i + 1] > g->count)
        {
            return CD_INVALID;
        }
    }
    for (unsigned i = 0; i < g->vertices; ++i)
    {
        for (unsigned j = g->offset[i]; j < g->offset[i + 1]; ++j)
        {
            if (g->edges[j].from != i || g->edges[j].to >= g->vertices)
            {
                return CD_INVALID;
            }
        }
    }
    uint32_t distance[8];
    unsigned queue[8], head = 0, tail = 1;
    for (unsigned i = 0; i < 8; ++i)
    {
        distance[i] = UINT32_MAX;
    }
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
    for (unsigned i = 0; i < 8; ++i)
    {
        out[i] = distance[i];
    }
    return CD_OK;
}
