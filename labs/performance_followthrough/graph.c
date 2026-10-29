#include "algo.h"
#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#define C(x)                                                  \
    do                                                        \
    {                                                         \
        if (!(x))                                             \
        {                                                     \
            fprintf(stderr, "PF_GRAPH_CHECK:%d\n", __LINE__); \
            exit(1);                                          \
        }                                                     \
    } while (0)

struct pf_ledger
{
    unsigned vertices, edges, attempted, accepted;
    uint64_t inspected;
    unsigned identities[32];
};

static struct pf_ledger query(const struct cd_csr *g, unsigned stop)
{
    uint32_t d[8];
    C(cd_csr_bfs(g, 0, d) == CD_OK);
    const uint8_t *a[] = {(const uint8_t *)"a", (const uint8_t *)"aaaa"};
    const size_t n[] = {1, 4};
    struct pf_ledger r = {0};
    for (unsigned v = 0; v < g->vertices; ++v)
    {
        if (d[v] != UINT32_MAX)
        {
            ++r.vertices;
            for (unsigned e = g->offset[v]; e < g->offset[v + 1]; ++e)
            {
                ++r.edges;
                unsigned to = g->edges[e].to;
                size_t at = 0;
                while (at < n[to])
                {
                    size_t match;
                    algo_work w = {0};
                    int status =
                        algo_match(a[to] + at, n[to] - at, (const uint8_t *)"a",
                                   1, &match, &w);
                    r.inspected += w.comparisons;
                    if (status == ALGO_MISSING)
                    {
                        break;
                    }
                    C(status == ALGO_OK);
                    ++r.attempted;
                    if (r.attempted == stop)
                    {
                        return r;
                    }
                    r.identities[r.accepted++] = e * 8 + (unsigned)(at + match);
                    at += match + 1;
                }
            }
        }
    }
    return r;
}

int main(void)
{
    struct cd_edge e[] = {{0, 1, 0}, {0, 1, 0}, {1, 0, 0}};
    struct cd_csr g;
    C(cd_csr_build(&g, 2, e, 3, 1) == CD_OK);
    struct pf_ledger full = query(&g, 0), partial = query(&g, 5);
    C(full.vertices == 2 && full.edges == 3 && full.attempted == 9 &&
      full.accepted == 9 && full.inspected == 9);
    C(partial.vertices == 1 && partial.edges == 2 && partial.attempted == 5 &&
      partial.accepted == 4);
    for (unsigned i = 0; i < partial.accepted; ++i)
    {
        C(full.identities[i] == partial.identities[i]);
    }
    struct cd_csr old = g;
    C(cd_csr_build(&g, 2, e, 3, 0) == CD_DUPLICATE);
    for (unsigned i = 0; i < 3; ++i)
    {
        C(g.edges[i].to == old.edges[i].to);
    }
    puts(
        "PF_GRAPH_PASS:V2:E3:Z9:byte-comparisons9:parallel-retained:stream-attempt5-accepted4-prefix");
    return 0;
}
