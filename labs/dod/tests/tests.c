#include "dod.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "DOD_CHECK:%s:%s\n", argv[1], #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct d_alloc
{
    unsigned calls, releases, fail;
};

static unsigned char *allocate(void *p, size_t n)
{
    struct d_alloc *a = p;
    a->calls++;
    return a->fail ? NULL : malloc(n);
}

static void release(void *p, unsigned char *q)
{
    struct d_alloc *a = p;
    a->releases++;
    free(q);
}

void d_histories(const char *);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "columns"))
    {
        struct d_columns c = {{1, 2, 3, 4}, {2, -1, 1, 0}, {7, 8, 9, 10}, 4};
        CHECK(d_columns_step(&c, 2) == D_OK);
        CHECK(c.x[0] == 5 && c.x[1] == 0 && c.x[2] == 5 && c.cold[3] == 10);
        c.x[3] = INT32_MAX;
        c.velocity[3] = 1;
        CHECK(d_columns_step(&c, 1) == D_RANGE && c.x[0] == 5 &&
              c.x[3] == INT32_MAX);
    }
    else if (!strcmp(argv[1], "small"))
    {
        struct d_alloc a = {0, 0, 0};
        struct d_small s, d;
        unsigned char bytes[17];
        size_t i;
        for (i = 0; i < 17; i++)
        {
            bytes[i] = (unsigned char)i;
        }
        d_small_init(&s, (struct d_allocator){&a, allocate, release});
        d_small_init(&d, (struct d_allocator){&a, allocate, release});
        CHECK(d_small_append(&s, NULL, 0) == D_OK && s.length == 0 &&
              a.calls == 0);
        CHECK(d_small_append(&s, bytes, 16) == D_OK && s.length == 16 &&
              a.calls == 0);
        a.fail = 1;
        CHECK(d_small_append(&s, bytes + 16, 1) == D_FAILED && s.length == 16 &&
              s.heap == NULL);
        a.fail = 0;
        CHECK(d_small_append(&s, bytes + 16, 1) == D_OK && s.length == 17 &&
              s.heap != NULL);
        CHECK(d_small_move(&d, &s) == D_OK && s.length == 0 && d.length == 17 &&
              a.releases == 0);
        CHECK(d_small_truncate(&d, 8) == D_OK && d_small_compact(&d) == D_OK &&
              d.heap == NULL && a.releases == 1);
        for (i = 0; i < 8; i++)
        {
            CHECK(d_small_data(&d)[i] == i);
        }
        d_small_reset(&s);
        d_small_reset(&d);
        CHECK(a.releases == 1);
    }
    else if (!strcmp(argv[1], "swar"))
    {
        unsigned a, b, lane;
        CHECK(d_swar_add(UINT32_C(0xff00ff00), UINT32_C(0x01010101)) ==
              UINT32_C(0xff01ff01));
        for (a = 0; a < 256; a++)
        {
            for (b = 0; b < 256; b++)
            {
                uint32_t x = (uint32_t)a * UINT32_C(0x01010101),
                         y = (uint32_t)b * UINT32_C(0x01010101),
                         want = (uint32_t)(a + b > 255 ? 255 : a + b) *
                                UINT32_C(0x01010101);
                CHECK(d_swar_add(x, y) == want);
                for (lane = 0; lane < 4; lane++)
                {
                    CHECK(
                        d_swar_add((uint32_t)a << (8 * lane),
                                   (uint32_t)b << (8 * lane)) ==
                        ((uint32_t)(a + b > 255 ? 255 : a + b) << (8 * lane)));
                }
            }
        }
    }
    else if (!strcmp(argv[1], "sparse"))
    {
        struct d_sparse s;
        struct d_handle a, b, c;
        int out = 99;
        unsigned i;
        d_sparse_init(&s);
        CHECK(d_sparse_add(&s, 11, &a) == D_OK);
        CHECK(d_sparse_add(&s, 22, &b) == D_OK);
        CHECK(d_sparse_add(&s, 33, &c) == D_OK);
        CHECK(d_sparse_remove(&s, b) == D_OK && s.count == 2 &&
              s.dense[1] == c.slot && s.rank[c.slot] == 1);
        CHECK(d_sparse_get(&s, b, &out) == D_STALE && out == 99);
        CHECK(d_sparse_get(&s, c, &out) == D_OK && out == 33);
        CHECK(d_sparse_remove(&s, a) == D_OK);
        for (i = 0; i < 2; i++)
        {
            CHECK(d_sparse_add(&s, 44, &a) == D_OK);
            CHECK(d_sparse_remove(&s, a) == D_OK);
        }
        CHECK(s.slots[0].generation == 4);
        CHECK(d_sparse_add(&s, 55, &a) == D_OK && a.slot != 0);
    }
    else if (!strcmp(argv[1], "ecs"))
    {
        struct d_ecs e;
        d_ecs_init(&e);
        CHECK(d_ecs_set(&e, 0, 1, 10, 0) == D_OK);
        CHECK(d_ecs_set(&e, 1, 1, 20, 0) == D_OK);
        CHECK(d_ecs_set(&e, 2, 3, 30, 2) == D_OK);
        CHECK(d_ecs_set(&e, 0, 3, 10, 1) == D_OK);
        CHECK(e.mask[1] == 1 && e.row[1] == 0 && e.tables[0].ids[0] == 1);
        CHECK(d_ecs_set(&e, 1, 3, 20, 4) == D_FULL && e.mask[1] == 1 &&
              e.tables[0].count == 1);
        CHECK(d_ecs_step(&e) == D_OK && e.tables[2].position[0] == 32 &&
              e.tables[2].position[1] == 11);
        e.tables[2].position[1] = INT_MAX;
        CHECK(d_ecs_step(&e) == D_RANGE && e.tables[2].position[0] == 32);
    }
    else if (!strcmp(argv[1], "persistent"))
    {
        struct d_tree t;
        unsigned a = 99, b = 99, n, out;
        int v;
        d_tree_init(&t);
        CHECK(d_tree_update(&t, 0, 3, 17, &a) == D_OK && t.used == 4);
        CHECK(d_tree_update(&t, a, 6, 29, &b) == D_OK && t.used == 8);
        CHECK(d_tree_get(&t, a, 6, &v) == D_OK && v == 0);
        CHECK(d_tree_get(&t, b, 3, &v) == D_OK && v == 17);
        for (n = 1; n <= 4; n++)
        {
            t.fail_after = n;
            out = 99;
            CHECK(d_tree_update(&t, b, 3, 50, &out) == D_FAILED && out == 99 &&
                  t.used == 8);
            CHECK(d_tree_get(&t, b, 3, &v) == D_OK && v == 17);
        }
        t.fail_after = 0;
        CHECK(d_tree_update(&t, b, 3, 50, &out) == D_OK);
        CHECK(d_tree_get(&t, b, 3, &v) == D_OK && v == 17);
        CHECK(d_tree_get(&t, out, 3, &v) == D_OK && v == 50);
    }
    else if (!strcmp(argv[1], "hamt"))
    {
        struct d_hamt h;
        unsigned a = 99, b = 99, c = 99, n, out;
        int v = 77;
        d_hamt_init(&h);
        CHECK(d_hamt_update(&h, 0, 1, 10, &a) == D_OK);
        CHECK(d_hamt_update(&h, a, 17, 20, &b) == D_OK);
        CHECK(d_hamt_get(&h, b, 1, &v) == D_OK && v == 10);
        CHECK(d_hamt_get(&h, b, 17, &v) == D_OK && v == 20);
        CHECK(d_hamt_get(&h, a, 17, &v) == D_STALE && v == 20);
        CHECK(d_hamt_update(&h, b, 0, 30, &c) == D_OK);
        CHECK(d_hamt_get(&h, c, 1, &v) == D_OK && v == 10);
        for (n = 1; n <= 3; n++)
        {
            h.fail_after = n;
            out = 99;
            CHECK(d_hamt_update(&h, c, 17, 88, &out) == D_FAILED && out == 99 &&
                  h.used == 9);
        }
        h.fail_after = 0;
        CHECK(d_hamt_update(&h, c, 17, 88, &out) == D_OK);
        CHECK(d_hamt_get(&h, c, 17, &v) == D_OK && v == 20);
        CHECK(d_hamt_get(&h, out, 17, &v) == D_OK && v == 88);
    }
    else if (!strcmp(argv[1], "rope"))
    {
        struct d_rope r;
        unsigned a = 99, b = 99, c = 99, d = 99;
        size_t n = 99;
        unsigned char out[8] = {42};
        d_rope_init(&r);
        CHECK(d_rope_leaf(&r, (const unsigned char *)"ab", 2, &a) == D_OK);
        CHECK(d_rope_leaf(&r, (const unsigned char *)"cd", 2, &b) == D_OK);
        CHECK(d_rope_concat(&r, a, b, &c) == D_OK);
        CHECK(d_rope_concat(&r, c, c, &d) == D_OK);
        CHECK(d_rope_flatten(&r, d, out, 7, &n) == D_FULL && n == 99 &&
              out[0] == 42);
        CHECK(d_rope_flatten(&r, d, out, 8, &n) == D_OK && n == 8 &&
              !memcmp(out, "abcdabcd", 8));
        CHECK(d_rope_flatten(&r, a, out, 8, &n) == D_OK && n == 2 &&
              !memcmp(out, "ab", 2));
    }
    else if (strcmp(argv[1], "oracle"))
    {
        return 2;
    }
    d_histories(argv[1]);
    return 0;
}
