#include "frontier.h"
#include <limits.h>
#define CHECK(x)                             \
    do                                       \
    {                                        \
        if (!(x))                            \
            return ag_failure(#x, __LINE__); \
    } while (0)

static int equal(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

static int checked(const struct cd_edge *edges, size_t n, unsigned vertices,
                   unsigned source, unsigned parallel, struct ag_result *r,
                   struct ag_trace *t)
{
    struct ag_workspace workspace;
    CHECK(ag_run(edges, n, vertices, source, parallel, &workspace, 8, r, t) ==
          AG_OK);
    struct cd_csr g;
    CHECK(cd_csr_build(&g, vertices, edges, n, parallel) == CD_OK);
    uint64_t distance[8];
    unsigned previous[8], queue[8];
    struct ad_traversal bfs;
    CHECK(ad_dijkstra(&g, source, 65534, distance, previous) == AD_OK &&
          ad_bfs(&g, source, 0, queue, 8, &bfs) == AD_OK);
    CHECK(t->queue_pushes == bfs.count && t->queue_pops == bfs.count &&
          t->bfs_count == bfs.count && t->heap_inserts == bfs.count &&
          t->heap_takes == bfs.count && t->weighted_count == bfs.count);
    unsigned done = 0;
    for (unsigned k = 0; k < t->weighted_count; ++k)
    {
        const struct ag_step *s = &t->weighted[k];
        CHECK(s->done == done && s->count && s->selected == s->frontier[0] &&
              !(done & (1u << s->selected)));
        unsigned open = 0;
        for (unsigned i = 0; i < s->count; ++i)
        {
            unsigned v = s->frontier[i];
            CHECK(v < vertices && !(open & (1u << v)) && !(done & (1u << v)) &&
                  s->distance[v] <= 65534 &&
                  s->priority[i] == (int)(s->distance[v] * 8u + v));
            open |= 1u << v;
            if (i)
                CHECK(s->priority[(i - 1) / 2] <= s->priority[i]);
        }
        CHECK(s->seen == (done | open));
        done |= 1u << s->selected;
    }
    struct ag_certificate c = {{0}, r->reachable, {0}};
    for (unsigned i = 0; i < 8; ++i)
    {
        if (distance[i] == UINT64_MAX)
            CHECK(!(r->reachable & (1u << i)) && r->distance[i] == 0 &&
                  r->hops[i] == 0 && r->previous[i] == 8);
        else
        {
            CHECK((r->reachable & (1u << i)) && r->distance[i] == distance[i] &&
                  r->hops[i] == bfs.distance[i] &&
                  r->previous[i] == (i == source ? 8 : previous[i]));
        }
        c.distance[i] = r->distance[i];
        c.previous[i] = i == source ? source : r->previous[i];
    }
    CHECK(ag_certificate(&g, source, &c) == AG_OK);
    return 0;
}

static int fixture(void)
{
    struct cd_edge edges[] = {
        {0, 1, 4}, {0, 2, 1}, {2, 1, 1}, {1, 3, 1}, {2, 3, 5}};
    struct ag_result r;
    struct ag_trace t;
    CHECK(!checked(edges, 5, 5, 0, 0, &r, &t));
    unsigned weighted[] = {0, 2, 1, 3}, bfs[] = {0, 1, 2, 3};
    for (unsigned i = 0; i < 4; ++i)
        CHECK(t.weighted[i].selected == weighted[i] &&
              t.bfs[i].selected == bfs[i]);
    CHECK(r.reachable == 15 && r.distance[1] == 2 && r.distance[2] == 1 &&
          r.distance[3] == 3 && r.hops[1] == 1 && r.hops[3] == 2 &&
          r.previous[1] == 2 && r.previous[3] == 1);
    CHECK(t.relaxations == 5 && t.heap_changes == 2 && t.heap_inserts == 4 &&
          t.heap_takes == 4);
    ag_emit(1000, 0, &r, &t);
    return 0;
}

static int boundaries(void)
{
    struct ag_workspace w;
    struct ag_result r = {0};
    struct ag_trace t = {0};
    r.distance[0] = 77;
    t.heap_takes = 77;
    struct ag_result old = r;
    struct ag_trace before = t;
    struct cd_edge bad[] = {{0, 1, 65530}, {1, 2, 10}};
    CHECK(ag_run(bad, 2, 3, 0, 0, &w, 8, &r, &t) == AG_RANGE &&
          equal(&r, &old, sizeof r) && equal(&t, &before, sizeof t));
    bad[1].weight = -1;
    CHECK(ag_run(bad, 2, 3, 0, 0, &w, 8, &r, &t) == AG_INVALID &&
          equal(&r, &old, sizeof r) && equal(&t, &before, sizeof t));
    bad[1] = (struct cd_edge){1, 3, 1};
    CHECK(ag_run(bad, 2, 3, 0, 0, &w, 8, &r, &t) == AG_INVALID &&
          equal(&r, &old, sizeof r));
    CHECK(ag_run(0, 0, 1, 0, 0, &w, 0, &r, &t) == AG_SPACE &&
          equal(&r, &old, sizeof r) && equal(&t, &before, sizeof t));
    CHECK(ag_run(0, 0, 0, 0, 0, &w, 8, &r, &t) == AG_INVALID &&
          equal(&r, &old, sizeof r));
    CHECK(ag_run(bad, 25, 3, 0, 0, &w, 8, &r, &t) == AG_INVALID &&
          equal(&r, &old, sizeof r));
    CHECK(!checked(0, 0, 1, 0, 0, &r, &t) && r.reachable == 1 &&
          t.weighted_count == 1);
    ag_emit(1001, 0, &r, &t);
    struct cd_edge zero[] = {
        {0, 0, 0}, {0, 2, 0}, {0, 1, 0}, {1, 2, 0}, {2, 1, 0}};
    CHECK(!checked(zero, 5, 4, 0, 0, &r, &t));
    CHECK(t.weighted[1].selected == 1 && t.weighted[2].selected == 2 &&
          r.previous[1] == 0 && r.previous[2] == 0);
    ag_emit(1002, 0, &r, &t);
    struct cd_edge exact[] = {{0, 1, 65534}, {1, 1, 0}};
    CHECK(!checked(exact, 2, 2, 0, 0, &r, &t) && r.distance[1] == 65534);
    ag_emit(1003, 0, &r, &t);
    struct cd_edge unreachable[] = {{2, 3, INT_MAX}};
    CHECK(!checked(unreachable, 1, 4, 0, 0, &r, &t) && r.reachable == 1);
    return 0;
}

static int parallel(void)
{
    struct cd_edge edges[] = {{0, 1, 9}, {0, 1, 4}, {0, 1, 2}, {1, 1, 0}};
    struct cd_edge saved[4];
    for (unsigned i = 0; i < 4; ++i)
        saved[i] = edges[i];
    struct ag_result r;
    struct ag_trace t;
    CHECK(!checked(edges, 4, 2, 0, 1, &r, &t));
    CHECK(r.distance[1] == 2 && r.hops[1] == 1 && t.heap_changes == 2 &&
          t.heap_inserts == 2 && t.relaxations == 3 &&
          equal(edges, saved, sizeof edges));
    ag_emit(1004, 0, &r, &t);
    struct ag_workspace w;
    struct ag_result old = r;
    struct ag_trace before = t;
    CHECK(ag_run(edges, 4, 2, 0, 0, &w, 8, &r, &t) == AG_INVALID &&
          equal(&r, &old, sizeof r) && equal(&t, &before, sizeof t));
    struct cd_edge tie[] = {{0, 2, 1}, {0, 1, 1}, {2, 3, 0}, {1, 3, 0}};
    CHECK(!checked(tie, 4, 4, 0, 0, &r, &t));
    CHECK(t.weighted[1].selected == 1 && r.previous[3] == 1);
    ag_emit(1005, 0, &r, &t);
    return 0;
}

static int certificates(void)
{
    struct cd_edge edges[] = {{0, 1, 5}, {0, 2, 1}, {2, 1, 1}};
    struct cd_csr g;
    CHECK(cd_csr_build(&g, 3, edges, 3, 0) == CD_OK);
    struct ag_certificate c = {{0, 5, 1}, 7, {0, 0, 0}};
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    c.distance[1] = 2;
    c.previous[1] = 2;
    CHECK(ag_certificate(&g, 0, &c) == AG_OK);
    c.previous[1] = 1;
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    c.previous[1] = 2;
    c.reachable = 5;
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    c.reachable = 7;
    c.distance[2] = INT64_MAX;
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    struct cd_edge negative[] = {{0, 1, 1}, {0, 2, 2}, {2, 1, -3}};
    CHECK(cd_csr_build(&g, 3, negative, 3, 0) == CD_OK);
    c = (struct ag_certificate){{0, -1, 2}, 7, {0, 2, 0}};
    CHECK(ag_certificate(&g, 0, &c) == AG_OK);
    c.distance[1] = 1;
    c.previous[1] = 0;
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    struct cd_edge cycle[] = {{0, 1, 0}, {1, 2, -1}, {2, 1, 0}};
    CHECK(cd_csr_build(&g, 3, cycle, 3, 0) == CD_OK);
    c = (struct ag_certificate){{0, 0, -1}, 7, {0, 0, 1}};
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    struct cd_edge triangle[] = {{0, 1, 2}, {1, 0, 2}, {0, 2, 3},
                                 {2, 0, 3}, {1, 2, 2}, {2, 1, 2}};
    CHECK(cd_csr_build(&g, 3, triangle, 6, 0) == CD_OK);
    c = (struct ag_certificate){{0, 2, 4}, 7, {0, 0, 1}};
    CHECK(ag_certificate(&g, 0, &c) == AG_BAD_CERTIFICATE);
    c.distance[2] = 3;
    c.previous[2] = 0;
    CHECK(ag_certificate(&g, 0, &c) == AG_OK);
    return 0;
}

static int exhaustive(void)
{
    for (unsigned code = 0; code < 729; ++code)
    {
        unsigned choices = code;
        struct cd_edge edges[6];
        size_t n = 0;
        for (unsigned from = 0; from < 3; ++from)
            for (unsigned to = 0; to < 3; ++to)
                if (from != to)
                {
                    unsigned choice = choices % 3;
                    choices /= 3;
                    if (choice)
                        edges[n++] =
                            (struct cd_edge){from, to, choice == 1 ? 0 : 2};
                }
        for (unsigned source = 0; source < 3; ++source)
        {
            struct ag_result r;
            struct ag_trace t;
            CHECK(!checked(edges, n, 3, source, 0, &r, &t));
            ag_emit(code, source, &r, &t);
        }
    }
    return 0;
}

int ag_test(unsigned which)
{
    switch (which)
    {
        case 0:
            return fixture();
        case 1:
            return boundaries();
        case 2:
            return parallel();
        case 3:
            return certificates();
        case 4:
            return exhaustive();
        default:
            return 2;
    }
}
