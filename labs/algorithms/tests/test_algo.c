#define _GNU_SOURCE
#include "algo.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ALGO_NATIVE_ASM
#include <sys/mman.h>
#include <unistd.h>
int algo_abi_insert_probe(const uint32_t *, uint32_t, uint32_t, uint32_t *);
int algo_abi_sift_probe(const uint32_t *, uint32_t, uint32_t, uint32_t *);
#endif
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)
static uint32_t seed = 0x81ad235fu;

static uint32_t rnd(void)
{
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

static int ring_test(void)
{
    algo_ring q;
    CHECK(algo_ring_init(&q, 3) == ALGO_OK);
    int32_t v = -1;
    CHECK(algo_ring_put(&q, 11) == 0 && algo_ring_put(&q, 22) == 0);
    CHECK(algo_ring_take(&q, &v) == 0 && v == 11);
    CHECK(algo_ring_put(&q, 33) == 0 && algo_ring_put(&q, 44) == 0);
    algo_ring before = q;
    CHECK(algo_ring_put(&q, 55) == ALGO_FULL && !memcmp(&q, &before, sizeof q));
    CHECK(algo_ring_take(&q, &v) == 0 && v == 22);
    CHECK(algo_ring_take(&q, &v) == 0 && v == 33);
    CHECK(algo_ring_take(&q, &v) == 0 && v == 44);
    CHECK(algo_ring_take(&q, &v) == ALGO_MISSING && v == 44);
    for (unsigned cycle = 0; cycle < 100; ++cycle)
    {
        CHECK(algo_ring_put(&q, (int32_t)cycle) == 0);
        CHECK(algo_ring_take(&q, &v) == 0 && v == (int32_t)cycle);
    }
    CHECK(algo_ring_init(&q, 0) == ALGO_BAD);
    return 0;
}

static int hash_test(void)
{
    algo_hash h;
    algo_hash_init(&h);
    int32_t v = -99;
    algo_work w = {0};
    CHECK(algo_hash_put(&h, 0, 1, &w) == 0 &&
          algo_hash_put(&h, 16, 2, &w) == 0 &&
          algo_hash_put(&h, 32, 3, &w) == 0);
    CHECK(algo_hash_delete(&h, 16, &w) == 0);
    CHECK(algo_hash_get(&h, 32, &v, &w) == 0 && v == 3);
    CHECK(algo_hash_put(&h, 32, 99, &w) == 0 && h.count == 2);
    CHECK(algo_hash_get(&h, 32, &v, 0) == 0 && v == 99);
    algo_hash_init(&h);
    uint8_t live[32] = {0};
    int32_t value[32] = {0};
    unsigned count = 0;
    for (unsigned step = 0; step < 6000; ++step)
    {
        unsigned k = rnd() % 32, op = rnd() % 3;
        int rc;
        if (op == 0)
        {
            int32_t x = (int32_t)(rnd() % 2000) - 1000;
            algo_hash old = h;
            rc = algo_hash_put(&h, k * 16, x, 0);
            if (live[k] || count < 16)
            {
                CHECK(rc == 0);
                if (!live[k])
                {
                    ++count;
                }
                live[k] = 1;
                value[k] = x;
            }
            else
            {
                CHECK(rc == ALGO_FULL && !memcmp(&old, &h, sizeof h));
            }
        }
        else if (op == 1)
        {
            rc = algo_hash_delete(&h, k * 16, 0);
            CHECK(rc == (live[k] ? ALGO_OK : ALGO_MISSING));
            if (live[k])
            {
                --count;
            }
            live[k] = 0;
        }
        else
        {
            v = 12345;
            rc = algo_hash_get(&h, k * 16, &v, 0);
            CHECK(rc == (live[k] ? ALGO_OK : ALGO_MISSING));
            CHECK(v == (live[k] ? value[k] : 12345));
        }
        CHECK(h.count == count);
        for (unsigned j = 0; j < 32; ++j)
        {
            rc = algo_hash_get(&h, j * 16, &v, 0);
            CHECK(rc == (live[j] ? ALGO_OK : ALGO_MISSING));
            if (live[j])
            {
                CHECK(v == value[j]);
            }
        }
    }
    CHECK(w.visits > 0);
    return 0;
}

static int avl_walk(const algo_avl *t, int p, int64_t low, int64_t high,
                    uint64_t *seen, unsigned *count)
{
    if (p < 0)
    {
        return 0;
    }
    if (p >= 64 || (*seen & (UINT64_C(1) << p)))
    {
        return -1000;
    }
    *seen |= UINT64_C(1) << p;
    const algo_node *n = &t->nodes[p];
    if (!n->live || n->key <= low || n->key >= high)
    {
        return -1000;
    }
    int a = avl_walk(t, n->left, low, n->key, seen, count),
        b = avl_walk(t, n->right, n->key, high, seen, count);
    if (a < 0 || b < 0 || abs(a - b) > 1 || n->height != 1 + (a > b ? a : b))
    {
        return -1000;
    }
    ++*count;
    return n->height;
}

static int avl_test(void)
{
    algo_avl t;
    algo_avl_init(&t);
    uint8_t live[96] = {0};
    unsigned count = 0;
    for (unsigned step = 0; step < 6000; ++step)
    {
        unsigned k = rnd() % 96;
        int32_t key = (int32_t)k - 48;
        int rc;
        if (rnd() & 1)
        {
            algo_avl old = t;
            rc = algo_avl_insert(&t, key);
            if (live[k] || count < 64)
            {
                CHECK(rc == 0);
                if (!live[k])
                {
                    ++count;
                }
                live[k] = 1;
            }
            else
            {
                CHECK(rc == ALGO_FULL && !memcmp(&old, &t, sizeof t));
            }
        }
        else
        {
            rc = algo_avl_delete(&t, key);
            CHECK(rc == (live[k] ? 0 : ALGO_MISSING));
            if (live[k])
            {
                --count;
            }
            live[k] = 0;
        }
        unsigned observed = 0;
        uint64_t seen = 0;
        CHECK(avl_walk(&t, t.root, INT64_MIN, INT64_MAX, &seen, &observed) >=
              0);
        CHECK(observed == count && t.count == count);
        for (unsigned j = 0; j < 64; ++j)
        {
            CHECK(!!t.nodes[j].live == !!(seen & (UINT64_C(1) << j)));
        }
        for (unsigned j = 0; j < 96; ++j)
        {
            CHECK(algo_avl_contains(&t, (int32_t)j - 48) == live[j]);
        }
    }
    algo_avl_init(&t);
    for (int i = 0; i < 64; ++i)
    {
        CHECK(algo_avl_insert(&t, i) == 0);
    }
    CHECK(algo_avl_insert(&t, 64) == ALGO_FULL);
    for (int i = 0; i < 64; ++i)
    {
        CHECK(algo_avl_delete(&t, i) == 0);
    }
    CHECK(t.root == -1 && t.count == 0);
    CHECK(algo_avl_insert(&t, INT32_MIN) == 0 &&
          algo_avl_insert(&t, INT32_MAX) == 0);
    return 0;
}

static int sets_test(void)
{
    algo_sets sets;
    CHECK(algo_sets_init(&sets, 32) == 0);
    unsigned label[32];
    for (unsigned i = 0; i < 32; ++i)
    {
        label[i] = i;
    }
    for (unsigned step = 0; step < 150; ++step)
    {
        unsigned a = rnd() % 32, b = rnd() % 32, from = label[b], to = label[a];
        CHECK(algo_sets_union(&sets, a, b) == 0);
        for (unsigned i = 0; i < 32; ++i)
        {
            if (label[i] == from)
            {
                label[i] = to;
            }
        }
        for (unsigned i = 0; i < 32; ++i)
        {
            for (unsigned j = 0; j < 32; ++j)
            {
                uint32_t x, y;
                CHECK(algo_sets_find(&sets, i, &x) == 0 &&
                      algo_sets_find(&sets, j, &y) == 0);
                CHECK((x == y) == (label[i] == label[j]));
            }
        }
    }
    CHECK(algo_sets_union(&sets, 32, 0) == ALGO_BAD);
    return 0;
}

static int graph_test(void)
{
    for (unsigned trial = 0; trial < 100; ++trial)
    {
        algo_graph g = {0};
        g.n = 1 + rnd() % 16;
        uint64_t fw[16][16], hop[16][16];
        for (unsigned i = 0; i < 16; ++i)
        {
            for (unsigned j = 0; j < 16; ++j)
            {
                g.weight[i][j] = (i < g.n && j < g.n && (rnd() % 4 == 0))
                                     ? rnd() % 20
                                     : ALGO_INF;
                fw[i][j] = i == j ? 0 : g.weight[i][j];
                hop[i][j] = i == j                       ? 0
                            : g.weight[i][j] == ALGO_INF ? ALGO_INF
                                                         : 1;
            }
        }
        for (unsigned k = 0; k < g.n; ++k)
        {
            for (unsigned i = 0; i < g.n; ++i)
            {
                for (unsigned j = 0; j < g.n; ++j)
                {
                    if (fw[i][k] + fw[k][j] < fw[i][j])
                    {
                        fw[i][j] = fw[i][k] + fw[k][j];
                    }
                    if (hop[i][k] + hop[k][j] < hop[i][j])
                    {
                        hop[i][j] = hop[i][k] + hop[k][j];
                    }
                }
            }
        }
        for (unsigned start = 0; start < g.n; ++start)
        {
            uint32_t d[16];
            algo_work w = {0};
            CHECK(algo_dijkstra(&g, start, d, &w) == 0);
            for (unsigned i = 0; i < g.n; ++i)
            {
                CHECK(d[i] == fw[start][i]);
            }
            CHECK(algo_bfs(&g, start, d, &w) == 0);
            for (unsigned i = 0; i < g.n; ++i)
            {
                CHECK(d[i] == hop[start][i]);
            }
        }
    }
    algo_graph g;
    g.n = 3;
    for (unsigned i = 0; i < 16; ++i)
    {
        for (unsigned j = 0; j < 16; ++j)
        {
            g.weight[i][j] = ALGO_INF;
        }
    }
    g.weight[0][1] = UINT32_MAX - 2;
    g.weight[1][2] = 2;
    uint32_t d[16], old[16];
    for (unsigned i = 0; i < 16; ++i)
    {
        d[i] = 0xdeadbeef;
    }
    memcpy(old, d, sizeof d);
    CHECK(algo_dijkstra(&g, 0, d, 0) == ALGO_OVERFLOW &&
          !memcmp(d, old, sizeof d));
    return 0;
}

static int pair_compare(const void *x, const void *y)
{
    const algo_pair *a = x, *b = y;
    return a->key != b->key   ? (a->key > b->key ? 1 : -1)
           : a->tag != b->tag ? (a->tag > b->tag ? 1 : -1)
                              : 0;
}
#ifdef ALGO_NATIVE_ASM
static int word_compare(const void *x, const void *y)
{
    uint32_t a = *(const uint32_t *)x, b = *(const uint32_t *)y;
    return (a > b) - (a < b);
}
#endif
static int sort_test(void)
{
    algo_pair fixture[] = {{3, 0}, {1, 1}, {3, 2}, {2, 3}, {1, 4}};
    algo_work w = {0};
    CHECK(algo_insertion(fixture, 5, 5, &w) == 0);
    CHECK(w.moves == 10 && w.comparisons == 9);
    CHECK(fixture[0].tag == 1 && fixture[1].tag == 4 && fixture[4].tag == 2);
    for (unsigned trial = 0; trial < 200; ++trial)
    {
        uint32_t n = rnd() % 129;
        algo_pair a[128], b[128], expected[128];
        for (unsigned i = 0; i < n; ++i)
        {
            a[i] = (algo_pair){(rnd() % 11) ^ (rnd() & 0x80000000u), i};
        }
        memcpy(b, a, n * sizeof *a);
        memcpy(expected, a, n * sizeof *a);
        qsort(expected, n, sizeof *a, pair_compare);
        CHECK(algo_insertion(a, n, n, 0) == 0 && algo_merge(b, n, n, 0) == 0);
        CHECK(!memcmp(a, expected, n * sizeof *a) &&
              !memcmp(b, expected, n * sizeof *a));
        uint32_t words[128], pos;
        for (unsigned i = 0; i < n; ++i)
        {
            words[i] = expected[i].key;
        }
        for (unsigned q = 0; q < 30; ++q)
        {
            uint32_t key = q == 0 ? UINT32_MAX : q == 1 ? 0 : rnd(), want = 0;
            while (want < n && words[want] < key)
            {
                ++want;
            }
            w = (algo_work){0};
            CHECK(algo_lower(words, n, key, &pos, &w) == 0 && pos == want);
            CHECK(w.comparisons <= 8);
        }
    }
    uint32_t pos = 123;
    CHECK(algo_lower(0, 0, 9, &pos, 0) == 0 && pos == 0);
    uint32_t bad[] = {2, 1};
    CHECK(algo_lower(bad, 2, 1, &pos, 0) == ALGO_BAD && pos == 0);
    algo_pair old[5];
    memcpy(old, fixture, sizeof old);
    CHECK(algo_insertion(fixture, 5, 4, 0) == ALGO_BAD &&
          !memcmp(old, fixture, sizeof old));
    return 0;
}

static int heap_test(void)
{
    uint32_t heap[] = {5, 7, 8, 3, 6, 2};
    algo_work w = {0};
    CHECK(algo_sift(heap, 6, 6, 0, &w) == 0 && w.comparisons == 3 &&
          w.moves == 1);
    uint32_t expected[] = {8, 7, 5, 3, 6, 2};
    CHECK(!memcmp(heap, expected, sizeof heap));
    uint32_t broken[] = {1, 3, 2, 7};
    uint32_t saved[4];
    memcpy(saved, broken, sizeof saved);
    CHECK(algo_sift(broken, 4, 4, 0, 0) == ALGO_BAD &&
          !memcmp(saved, broken, sizeof saved));
    uint32_t ancestor_bad[] = {10, 1, 9, 20, 0};
    const uint32_t ancestor_before[] = {10, 1, 9, 20, 0};
    CHECK(algo_sift(ancestor_bad, 5, 5, 1, 0) == ALGO_BAD &&
          !memcmp(ancestor_bad, ancestor_before, sizeof ancestor_bad));
    uint32_t internal[] = {30, 1, 9, 20, 0};
    const uint32_t internal_expected[] = {30, 20, 9, 1, 0};
    CHECK(algo_sift(internal, 5, 5, 1, 0) == ALGO_OK &&
          !memcmp(internal, internal_expected, sizeof internal));
    return 0;
}

static int match_test(void)
{
    for (unsigned n = 0; n <= 8; ++n)
    {
        for (unsigned bits = 0; bits < (1u << n); ++bits)
        {
            uint8_t text[8];
            for (unsigned i = 0; i < n; ++i)
            {
                text[i] = (bits >> i) & 1;
            }
            for (unsigned m = 0; m <= 4; ++m)
            {
                for (unsigned pb = 0; pb < (1u << m); ++pb)
                {
                    uint8_t pattern[4];
                    for (unsigned i = 0; i < m; ++i)
                    {
                        pattern[i] = (pb >> i) & 1;
                    }
                    size_t expected = SIZE_MAX;
                    for (unsigned i = 0; i <= n; ++i)
                    {
                        if (m <= n - i && !memcmp(text + i, pattern, m))
                        {
                            expected = i;
                            break;
                        }
                    }
                    size_t found = SIZE_MAX;
                    int rc = algo_match(text, n, pattern, m, &found, 0);
                    CHECK(rc == (expected == SIZE_MAX ? ALGO_MISSING : 0) &&
                          found == expected);
                }
            }
        }
    }
    size_t out = 5;
    CHECK(algo_match(0, 0, 0, 0, &out, 0) == 0 && out == 0);
    CHECK(algo_match(0, 1, 0, 0, &out, 0) == ALGO_BAD);
    return 0;
}

static int arena_test(void)
{
    uint8_t bytes[128];
    algo_arena a;
    CHECK(algo_arena_init(&a, bytes + 1, 127) == 0);
    size_t off = SIZE_MAX;
    CHECK(algo_arena_take(&a, 7, 16, &off) == 0);
    CHECK(((uintptr_t)(a.data + off) & 15) == 0);
    size_t mark = a.used;
    CHECK(algo_arena_take(&a, 20, 8, &off) == 0);
    CHECK(algo_arena_rewind(&a, mark) == 0);
    algo_arena old = a;
    off = 99;
    CHECK(algo_arena_take(&a, SIZE_MAX, 8, &off) == ALGO_FULL && off == 99 &&
          !memcmp(&old, &a, sizeof a));
    CHECK(algo_arena_take(&a, 1, 3, &off) == ALGO_BAD);
    CHECK(algo_arena_rewind(&a, a.used + 1) == ALGO_BAD);
    return 0;
}

static int pool_test(void)
{
    algo_pool p;
    algo_pool_init(&p);
    uint64_t handles[16], h;
    int32_t v;
    for (unsigned i = 0; i < 16; ++i)
    {
        CHECK(algo_pool_take(&p, (int32_t)i, &handles[i]) == 0);
    }
    CHECK(algo_pool_take(&p, 99, &h) == ALGO_FULL);
    CHECK(algo_pool_release(&p, handles[4]) == 0);
    CHECK(algo_pool_take(&p, 44, &h) == 0 && h != handles[4]);
    CHECK(algo_pool_get(&p, handles[4], &v) == ALGO_MISSING);
    CHECK(algo_pool_get(&p, h, &v) == 0 && v == 44);
    CHECK(algo_pool_release(&p, handles[4]) == ALGO_MISSING);
    algo_pool_init(&p);
    p.generation[0] = UINT32_MAX;
    CHECK(algo_pool_take(&p, 1, &h) == 0);
    CHECK(algo_pool_release(&p, h) == 0 && p.state[0] == 2);
    CHECK(algo_pool_take(&p, 2, &h) == 0 && (uint32_t)h == 1);
    return 0;
}

static int sum_test(void)
{
    double a[] = {1e16, 1, -1e16}, sum = 0;
    CHECK(algo_sum(a, 3, &sum) == 0 && sum == 1);
    double b[] = {DBL_MAX, DBL_MAX};
    sum = 5;
    CHECK(algo_sum(b, 2, &sum) == ALGO_OVERFLOW && sum == 5);
    double nan = NAN;
    CHECK(algo_sum(&nan, 1, &sum) == ALGO_BAD && sum == 5);
    return 0;
}

static int q7_test(void)
{
    for (int x = -128; x < 128; ++x)
    {
        for (int y = -128; y < 128; ++y)
        {
            int8_t ax = (int8_t)x, by = (int8_t)y, out;
            int product = x * y;
            double scaled = (double)product / 128.0;
            long want = scaled < 0 ? -(long)floor(-scaled + 0.5)
                                   : (long)floor(scaled + 0.5);
            if (want > 127)
            {
                want = 127;
            }
            if (want < -128)
            {
                want = -128;
            }
            CHECK(algo_q7(&ax, &by, 1, &out) == 0 && out == want);
        }
    }
    int8_t x[16], y[16], out = 17;
    memset(x, -128, 16);
    memset(y, -128, 16);
    CHECK(algo_q7(x, y, 16, &out) == 0 && out == 127);
    CHECK(algo_q7(x, y, 17, &out) == ALGO_BAD && out == 127);
    return 0;
}

static int alu_test(void)
{
    for (unsigned a = 0; a < 16; ++a)
    {
        for (unsigned b = 0; b < 16; ++b)
        {
            for (unsigned c = 0; c < 2; ++c)
            {
                algo_alu_result r =
                    algo_alu4((uint8_t)a, (uint8_t)b, (uint8_t)c);
                int sa = a < 8 ? (int)a : (int)a - 16,
                    sb = b < 8 ? (int)b : (int)b - 16;
                int s = sa + sb + (int)c;
                CHECK(r.value == ((a + b + c) & 15) &&
                      r.carry == ((a + b + c) >> 4) &&
                      r.overflow == (s < -8 || s > 7));
            }
        }
    }
    return 0;
}

static int cache_test(void)
{
    uint32_t addresses[] = {0, 16, 0, 64, 0, 16, 80, 16, 0};
    algo_cache_result r;
    CHECK(algo_cache(addresses, 9, &r) == 0);
    CHECK(r.hits == 3 && r.misses == 6 && r.compulsory == 4 &&
          r.conflict == 2 && r.capacity == 0);
    uint32_t second[] = {0, 16, 32, 48, 64, 0};
    CHECK(algo_cache(second, 6, &r) == 0 && r.capacity == 1 &&
          r.compulsory == 5);
    CHECK(algo_cache(0, 0, &r) == 0 && r.hits == 0 && r.misses == 0);
    return 0;
}
#ifdef ALGO_NATIVE_ASM
static int assembly_test(int which)
{
    uint32_t out;
    /* Detect and restore a broken callee before any direct C-to-kernel call. */
    CHECK(algo_abi_probe(0, 0, 5, &out) == 1 && out == 0);
    uint32_t bad[] = {2, 1};
    out = 99;
    CHECK(algo_checked_lower(bad, 2, 2, 1, &out) == ALGO_BAD && out == 99);
    CHECK(algo_checked_lower(bad, 2, 1, 1, &out) == ALGO_BAD && out == 99);
    CHECK(algo_checked_lower(0, 0, 0, 1, &out) == 0 && out == 0);
    if (which == 0 || which == 2)
    {
        algo_pair short_array[2] = {{2, 0}, {1, 1}}, saved[2];
        memcpy(saved, short_array, sizeof saved);
        CHECK(algo_checked_insert(short_array, 2, 1) == ALGO_BAD &&
              !memcmp(saved, short_array, sizeof saved));
        CHECK(algo_checked_insert(short_array, 2, 2) == 0 &&
              short_array[0].tag == 1);
    }
    if (which == 0 || which == 3)
    {
        uint32_t broken[] = {1, 2, 3, 9};
        CHECK(algo_checked_sift(broken, 4, 4, 0) == ALGO_BAD && broken[0] == 1);
        uint32_t ancestor_bad[] = {10, 1, 9, 20, 0};
        const uint32_t ancestor_before[] = {10, 1, 9, 20, 0};
        CHECK(algo_checked_sift(ancestor_bad, 5, 5, 1) == ALGO_BAD &&
              !memcmp(ancestor_bad, ancestor_before, sizeof ancestor_bad));
        uint32_t internal[] = {30, 1, 9, 20, 0};
        const uint32_t internal_expected[] = {30, 20, 9, 1, 0};
        CHECK(algo_checked_sift(internal, 5, 5, 1) == ALGO_OK &&
              !memcmp(internal, internal_expected, sizeof internal));
    }
    CHECK(algo_asm_lower(0, 0, 5) == 0);
    CHECK(algo_abi_probe(0, 0, 5, &out) == 1 && out == 0);
    for (unsigned trial = 0; trial < 1000; ++trial)
    {
        unsigned n = rnd() % 33;
        algo_pair a[32], expected[32];
        uint32_t words[32];
        for (unsigned i = 0; i < n; ++i)
        {
            a[i] = (algo_pair){rnd() % 9 ^ (rnd() & 0x80000000u), i};
        }
        memcpy(expected, a, n * sizeof *a);
        qsort(expected, n, sizeof *a, pair_compare);
        if (which == 0 || which == 2)
        {
            CHECK(algo_abi_insert_probe((uint32_t *)a, n, 0, &out) == 1);
            CHECK(!memcmp(a, expected, n * sizeof *a));
        }
        for (unsigned i = 0; i < n; ++i)
        {
            words[i] = expected[i].key;
        }
        for (unsigned k = 0; k < 10; ++k)
        {
            uint32_t key = k == 0 ? UINT32_MAX : rnd(), want = 0;
            while (want < n && words[want] < key)
            {
                ++want;
            }
            CHECK(algo_abi_probe(words, n, key, &out) == 1 && out == want);
        }
        if (which == 0 || which == 3)
        {
            uint32_t heap[33], sorted[33], after[33];
            for (unsigned i = 0; i < 33; ++i)
            {
                heap[i] = 1000 - i;
            }
            heap[0] = rnd() % 1000;
            memcpy(sorted, heap, sizeof heap);
            qsort(sorted, 33, 4, word_compare);
            CHECK(algo_abi_sift_probe(heap, 33, 0, &out) == 1);
            for (unsigned i = 1; i < 33; ++i)
            {
                CHECK(heap[(i - 1) / 2] >= heap[i]);
            }
            memcpy(after, heap, sizeof heap);
            qsort(after, 33, 4, word_compare);
            CHECK(!memcmp(after, sorted, sizeof heap));
        }
    }
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0);
    uint8_t *m = mmap(0, (size_t)page * 2, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(m != MAP_FAILED);
    CHECK(mprotect(m + page, (size_t)page, PROT_NONE) == 0);
    uint32_t *last = (uint32_t *)(m + page - 4);
    *last = UINT32_MAX;
    CHECK(algo_asm_lower(last, 1, UINT32_MAX) == 0);
    CHECK(algo_asm_lower(last + 1, 0, 9) == 0);
    CHECK(munmap(m, (size_t)page * 2) == 0);
    uint32_t large[1024];
    for (unsigned i = 0; i < 1024; ++i)
    {
        large[i] = 0x80000000u;
    }
    CHECK(algo_asm_lower(large, 1024, UINT32_MAX) == 1024);
    CHECK(algo_asm_lower(large, 1024, 0x80000000u) == 0);
    return 0;
}
#endif
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "ring"))
    {
        return ring_test();
    }
    if (!strcmp(argv[1], "hash"))
    {
        return hash_test();
    }
    if (!strcmp(argv[1], "avl"))
    {
        return avl_test();
    }
    if (!strcmp(argv[1], "graph"))
    {
        return graph_test();
    }
    if (!strcmp(argv[1], "sets"))
    {
        return sets_test();
    }
    if (!strcmp(argv[1], "sort"))
    {
        return sort_test();
    }
    if (!strcmp(argv[1], "heap"))
    {
        return heap_test();
    }
    if (!strcmp(argv[1], "match"))
    {
        return match_test();
    }
    if (!strcmp(argv[1], "lifetime"))
    {
        return arena_test() || pool_test();
    }
    if (!strcmp(argv[1], "arena"))
    {
        return arena_test();
    }
    if (!strcmp(argv[1], "pool"))
    {
        return pool_test();
    }
    if (!strcmp(argv[1], "numeric"))
    {
        return sum_test() || q7_test();
    }
    if (!strcmp(argv[1], "sum"))
    {
        return sum_test();
    }
    if (!strcmp(argv[1], "q7"))
    {
        return q7_test();
    }
    if (!strcmp(argv[1], "models"))
    {
        return alu_test() || cache_test();
    }
    if (!strcmp(argv[1], "alu"))
    {
        return alu_test();
    }
    if (!strcmp(argv[1], "cache"))
    {
        return cache_test();
    }
#ifdef ALGO_NATIVE_ASM
    if (!strcmp(argv[1], "assembly"))
    {
        return assembly_test(0);
    }
    if (!strcmp(argv[1], "assembly_lower"))
    {
        return assembly_test(1);
    }
    if (!strcmp(argv[1], "assembly_insert"))
    {
        return assembly_test(2);
    }
    if (!strcmp(argv[1], "assembly_sift"))
    {
        return assembly_test(3);
    }
#endif
    return 2;
}
