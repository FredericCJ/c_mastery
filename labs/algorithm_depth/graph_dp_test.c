#include "algorithms.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "ALGORITHM_ASSERT graph_dp line%d: %s\n", \
                    __LINE__, #x);                                    \
            return 1;                                                 \
        }                                                             \
    } while (0)

static int graph_cases(void)
{
    struct cd_edge e[] = {{0, 1, 2}, {0, 2, 5}, {1, 2, 1}, {2, 3, 3}};
    struct cd_csr g;
    CHECK(cd_csr_build(&g, 5, e, 4, 1) == CD_OK);
    unsigned queue[8], order[8], previous[8];
    struct ad_frame frames[8];
    struct ad_traversal bfs, dfs;
    uint64_t distance[8];
    CHECK(ad_bfs(&g, 0, 0, queue, 8, &bfs) == AD_OK && bfs.count == 4 &&
          bfs.distance[3] == 2 && bfs.distance[4] == UINT32_MAX);
    CHECK(ad_dfs(&g, 0, 0, frames, 8, &dfs) == AD_OK && dfs.count == 4 &&
          dfs.finished == 4 && dfs.finish[0] == 3 && dfs.finish[3] == 0 &&
          dfs.peak == 4);
    CHECK(ad_bfs(&g, 0, 1, queue, 8, &bfs) == AD_OK && bfs.count == 5 &&
          bfs.roots == 2 && bfs.root[4] == 4);
    CHECK(ad_dfs(&g, 2, 1, frames, 8, &dfs) == AD_OK && dfs.count == 5 &&
          dfs.roots == 3 && dfs.root[0] == 0 && dfs.root[4] == 4);
    CHECK(ad_topological(&g, order) == AD_OK && order[0] == 0 && order[4] == 4);
    CHECK(ad_dijkstra(&g, 0, 100, distance, previous) == AD_OK &&
          distance[0] == 0 && distance[1] == 2 && distance[2] == 3 &&
          distance[3] == 6 && distance[4] == UINT64_MAX);
    struct ad_traversal saved = dfs;
    CHECK(ad_dfs(&g, 0, 0, frames, 4, &dfs) == AD_SPACE &&
          !memcmp(&saved, &dfs, sizeof dfs));
    uint64_t old[8];
    memcpy(old, distance, sizeof old);
    CHECK(ad_dijkstra(&g, 0, 5, distance, previous) == AD_OVERFLOW &&
          !memcmp(old, distance, sizeof old));
    g.edges[0].weight = -1;
    CHECK(ad_dijkstra(&g, 0, 100, distance, previous) == AD_INVALID &&
          !memcmp(old, distance, sizeof old));
    g.edges[0].weight = 2;
    g.offset[1] = 33;
    CHECK(ad_bfs(&g, 0, 0, queue, 8, &bfs) == AD_INVALID);
    struct cd_edge duplicate[] = {{0, 1, 1}, {0, 1, 1}, {0, 2, 1},
                                  {1, 3, 1}, {2, 3, 1}, {3, 0, 1}};
    CHECK(cd_csr_build(&g, 4, duplicate, 6, 1) == CD_OK);
    CHECK(ad_bfs(&g, 0, 0, queue, 4, &bfs) == AD_OK && bfs.count == 4 &&
          bfs.edges == 6 && bfs.peak <= 4);
    for (unsigned i = 0; i < 8; ++i)
    {
        order[i] = 99;
    }
    CHECK(ad_topological(&g, order) == AD_FAILED);
    for (unsigned i = 0; i < 8; ++i)
        CHECK(order[i] == 99);
    struct cd_edge path[7];
    for (unsigned i = 0; i < 7; ++i)
        path[i] = (struct cd_edge){i, i + 1, 1};
    CHECK(cd_csr_build(&g, 8, path, 7, 1) == CD_OK);
    CHECK(ad_dfs(&g, 0, 0, frames, 7, &dfs) == AD_SPACE);
    CHECK(ad_dfs(&g, 0, 0, frames, 8, &dfs) == AD_OK && dfs.peak == 8);
    CHECK(cd_csr_build(&g, 0, NULL, 0, 1) == CD_OK);
    CHECK(ad_bfs(&g, 0, 1, NULL, 0, &bfs) == AD_OK && !bfs.count);
    CHECK(ad_dfs(&g, 0, 1, NULL, 0, &dfs) == AD_OK && !dfs.count);
    CHECK(ad_topological(&g, order) == AD_OK);
    return 0;
}

static int all_graphs(void)
{
    for (unsigned mask = 0; mask < 4096; ++mask)
    {
        struct cd_edge edges[12];
        unsigned m = 0, bit = 0, reach[4][4] = {{0}};
        uint32_t shortest[4][4];
        for (unsigned i = 0; i < 4; ++i)
            for (unsigned j = 0; j < 4; ++j)
            {
                shortest[i][j] = i == j ? 0 : 99;
                if (i != j)
                {
                    if (mask & (1u << bit))
                    {
                        edges[m++] = (struct cd_edge){i, j, 1};
                        reach[i][j] = 1;
                        shortest[i][j] = 1;
                    }
                    ++bit;
                }
            }
        for (unsigned k = 0; k < 4; ++k)
            for (unsigned i = 0; i < 4; ++i)
                for (unsigned j = 0; j < 4; ++j)
                {
                    reach[i][j] |= reach[i][k] && reach[k][j];
                    if (shortest[i][k] + shortest[k][j] < shortest[i][j])
                        shortest[i][j] = shortest[i][k] + shortest[k][j];
                }
        struct cd_csr g;
        CHECK(cd_csr_build(&g, 4, edges, m, 1) == CD_OK);
        unsigned order[8], position[8], cyclic = 0;
        for (unsigned i = 0; i < 4; ++i)
            cyclic |= reach[i][i];
        enum ad_status rc = ad_topological(&g, order);
        CHECK(rc == (cyclic ? AD_FAILED : AD_OK));
        if (!cyclic)
        {
            for (unsigned i = 0; i < 4; ++i)
                position[order[i]] = i;
            for (unsigned i = 0; i < m; ++i)
                CHECK(position[edges[i].from] < position[edges[i].to]);
        }
        for (unsigned s = 0; s < 4; ++s)
        {
            unsigned queue[8];
            struct ad_frame frames[8];
            struct ad_traversal a, b;
            CHECK(ad_bfs(&g, s, 0, queue, 8, &a) == AD_OK);
            CHECK(ad_dfs(&g, s, 0, frames, 8, &b) == AD_OK);
            unsigned seen_a = 0, seen_b = 0;
            for (unsigned i = 0; i < a.count; ++i)
            {
                CHECK(!(seen_a & (1u << a.order[i])));
                seen_a |= 1u << a.order[i];
            }
            for (unsigned i = 0; i < b.count; ++i)
            {
                CHECK(!(seen_b & (1u << b.order[i])));
                seen_b |= 1u << b.order[i];
            }
            CHECK(seen_a == seen_b);
            unsigned reached;
            CHECK(ad_reachable_stack(&g, s, queue, 8, &reached) == AD_OK &&
                  reached == seen_a);
            for (unsigned v = 0; v < 4; ++v)
            {
                CHECK(!!(seen_a & (1u << v)) == (v == s || reach[s][v]));
                CHECK(a.distance[v] ==
                      (shortest[s][v] == 99 ? UINT32_MAX : shortest[s][v]));
            }
            CHECK(b.finished == b.count && b.peak <= 4);
        }
    }
    for (unsigned code = 0; code < 4096; ++code)
    {
        unsigned digits = code, m = 0;
        struct cd_edge edges[6];
        uint64_t oracle[3][3];
        for (unsigned i = 0; i < 3; ++i)
            for (unsigned j = 0; j < 3; ++j)
            {
                oracle[i][j] = i == j ? 0 : 99;
                if (i != j)
                {
                    unsigned option = digits % 4;
                    digits /= 4;
                    if (option)
                    {
                        edges[m++] = (struct cd_edge){i, j, (int)option - 1};
                        oracle[i][j] = option - 1;
                    }
                }
            }
        for (unsigned k = 0; k < 3; ++k)
            for (unsigned i = 0; i < 3; ++i)
                for (unsigned j = 0; j < 3; ++j)
                    if (oracle[i][k] + oracle[k][j] < oracle[i][j])
                        oracle[i][j] = oracle[i][k] + oracle[k][j];
        struct cd_csr g;
        CHECK(cd_csr_build(&g, 3, edges, m, 1) == CD_OK);
        for (unsigned s = 0; s < 3; ++s)
        {
            uint64_t distance[8];
            unsigned previous[8];
            CHECK(ad_dijkstra(&g, s, 100, distance, previous) == AD_OK);
            for (unsigned v = 0; v < 3; ++v)
            {
                CHECK(distance[v] ==
                      (oracle[s][v] == 99 ? UINT64_MAX : oracle[s][v]));
                if (distance[v] != UINT64_MAX)
                {
                    unsigned x = v, steps = 0;
                    uint64_t cost = 0;
                    while (x != s)
                    {
                        CHECK(++steps <= 3);
                        unsigned p = previous[x], found = 0;
                        for (unsigned i = 0; i < m; ++i)
                            if (edges[i].from == p && edges[i].to == x)
                            {
                                cost += (unsigned)edges[i].weight;
                                found = 1;
                                break;
                            }
                        CHECK(found);
                        x = p;
                    }
                    CHECK(cost == distance[v]);
                }
            }
        }
    }
    return 0;
}

static uint32_t subset(const struct ad_item *p, unsigned n, unsigned capacity,
                       unsigned exact, unsigned *found)
{
    uint32_t best = 0;
    *found = 0;
    for (unsigned mask = 0; mask < (1u << n); ++mask)
    {
        unsigned weight = 0;
        uint32_t value = 0;
        for (unsigned i = 0; i < n; ++i)
            if (mask & (1u << i))
            {
                weight += p[i].weight;
                value += p[i].value;
            }
        if ((exact ? weight == capacity : weight <= capacity) &&
            (!*found || value > best))
        {
            *found = 1;
            best = value;
        }
    }
    return best;
}

static int dp_cases(void)
{
    struct ad_interval intervals[] = {
        {0, 3, 0}, {1, 2, 1}, {2, 4, 2}, {3, 5, 3}};
    unsigned chosen[8];
    size_t count = 99;
    CHECK(ad_intervals(intervals, 4, chosen, &count) == AD_OK && count == 2 &&
          chosen[0] == 1 && chosen[1] == 2);
    /* All ordered four-interval tuples from the six positive subintervals of [0,3]. */
    struct ad_interval choices[] = {{0, 1, 0}, {0, 2, 0}, {0, 3, 0},
                                    {1, 2, 0}, {1, 3, 0}, {2, 3, 0}};
    for (unsigned code = 0; code < 1296; ++code)
    {
        unsigned rest = code;
        for (unsigned i = 0; i < 4; ++i)
        {
            intervals[i] = choices[rest % 6];
            intervals[i].label = i;
            rest /= 6;
        }
        CHECK(ad_intervals(intervals, 4, chosen, &count) == AD_OK);
        unsigned best = 0;
        for (unsigned mask = 0; mask < 16; ++mask)
        {
            unsigned used = 0, okay = 1;
            for (unsigned i = 0; i < 4; ++i)
                if (mask & (1u << i))
                {
                    ++used;
                    for (unsigned j = i + 1; j < 4; ++j)
                        if ((mask & (1u << j)) &&
                            intervals[i].start < intervals[j].end &&
                            intervals[j].start < intervals[i].end)
                            okay = 0;
                }
            if (okay && used > best)
                best = used;
        }
        CHECK(count == best);
        for (size_t i = 1; i < count; ++i)
            CHECK(intervals[chosen[i - 1]].end <= intervals[chosen[i]].start);
    }
    struct ad_item items[] = {{2, 3}, {3, 4}, {4, 5}};
    uint32_t result = 99;
    CHECK(ad_knapsack(items, 3, 5, 0, 0, &result) == AD_OK && result == 7);
    CHECK(ad_knapsack(items, 1, 4, 0, 0, &result) == AD_OK && result == 3);
    CHECK(ad_knapsack(items, 1, 4, 0, 1, &result) == AD_OK && result == 6);
    result = 99;
    CHECK(ad_knapsack(items, 1, 3, 1, 0, &result) == AD_MISSING &&
          result == 99);
    CHECK(ad_knapsack(items, 1, 4, 1, 1, &result) == AD_OK && result == 6);
    for (unsigned code = 0; code < 729; ++code)
    {
        unsigned rest = code;
        for (unsigned i = 0; i < 3; ++i)
        {
            items[i] = (struct ad_item){1 + rest % 3, (rest / 3) % 3};
            rest /= 9;
        }
        for (unsigned c = 0; c <= 8; ++c)
            for (unsigned exact = 0; exact < 2; ++exact)
            {
                unsigned found;
                uint32_t expected = subset(items, 3, c, exact, &found);
                result = 99;
                CHECK(ad_knapsack(items, 3, c, exact, 0, &result) ==
                      (found ? AD_OK : AD_MISSING));
                CHECK(result == (found ? expected : 99));
            }
    }
    struct ad_item overflow[] = {{1, UINT32_MAX}, {1, 1}};
    result = 99;
    CHECK(ad_knapsack(overflow, 2, 2, 0, 0, &result) == AD_OVERFLOW &&
          result == 99);
    struct ad_item zero = {0, 3};
    CHECK(ad_knapsack(&zero, 1, 2, 0, 0, &result) == AD_INVALID);
    return 0;
}

int main(void)
{
    if (graph_cases() || all_graphs() || dp_cases())
        return 1;
    puts(
        "ALGORITHM_GRAPH_DP_PASS digraphs=4096 weighted_graphs=4096 interval_tuples=1296 item_tuples=729");
    return 0;
}
