#include "dod_dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "dictionary";
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "DOD_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static unsigned lower(unsigned c)
{
    return c >= 65 && c <= 90 ? c + 32 : c;
}

static int eq(void *context, struct dh_key a, struct dh_key b)
{
    (void)context;
    return lower(a.bytes[0]) == lower(b.bytes[0]) && a.bytes[1] == b.bytes[1];
}

static uint32_t hash(void *context, struct dh_key key)
{
    return *(int *)context ? 0
                           : (uint32_t)lower(key.bytes[0]) << 24 | key.bytes[1];
}

static struct dh_key key(unsigned c, unsigned tag)
{
    struct dh_key k = {{(unsigned char)c, (unsigned char)tag}};
    return k;
}

static void invariant(const struct dh_map *m, unsigned root, unsigned depth)
{
    if (!root)
    {
        return;
    }
    CHECK(root <= m->used);
    const struct dh_map_node *n = &m->nodes[root];
    if (depth == 16)
    {
        CHECK(n->count > 0 && n->count <= 8 && n->bitmap == 0);
        for (unsigned i = 0; i < n->count; ++i)
        {
            for (unsigned j = i + 1; j < n->count; ++j)
            {
                CHECK(!eq(NULL, n->keys[i], n->keys[j]));
            }
        }
        return;
    }
    unsigned count = 0;
    for (unsigned slot = 0; slot < 4; ++slot)
    {
        count += (n->bitmap >> slot) & 1u;
    }
    CHECK(count == n->count && count > 0 && count <= 4 && n->bitmap < 16);
    for (unsigned i = 0; i < count; ++i)
    {
        CHECK(n->children[i] && n->children[i] < root);
        invariant(m, n->children[i], depth + 1);
    }
}

static int value(const struct dh_map *m, unsigned root, struct dh_key k)
{
    int v = -999;
    int s = d_map_get(m, root, k, &v, NULL);
    CHECK(s == D_OK || s == D_STALE);
    if (s == D_STALE)
    {
        CHECK(v == -999);
    }
    return v;
}

static void checks(void)
{
    for (int mode = 0; mode < 2; ++mode)
    {
        struct dh_map m;
        CHECK(d_map_init(&m, (struct dh_dictionary){&mode, hash, eq}) == D_OK);
        unsigned first = 0, second = 0, third = 0;
        struct dh_metrics cost;
        CHECK(d_map_update(&m, 0, key(65, 0), 10, &first, &cost) == D_OK &&
              cost.clones == 17);
        CHECK(d_map_update(&m, first, key(66, 0), 20, &second, &cost) == D_OK);
        CHECK(value(&m, first, key(66, 0)) == -999 &&
              value(&m, second, key(65, 0)) == 10 &&
              value(&m, second, key(66, 0)) == 20);
        CHECK(d_map_update(&m, second, key(97, 0), 30, &third, &cost) == D_OK);
        CHECK(value(&m, third, key(65, 0)) == 30 &&
              value(&m, second, key(97, 0)) == 10);
        invariant(&m, third, 0);
        CHECK(d_map_delete(&m, third, key(97, 0), &third, &cost) == D_OK &&
              value(&m, third, key(65, 0)) == -999 &&
              value(&m, third, key(66, 0)) == 20);
        CHECK(value(&m, second, key(65, 0)) == 10);
        invariant(&m, third, 0);
        unsigned before = m.used, unchanged = 99;
        CHECK(d_map_delete(&m, third, key(90, 9), &unchanged, &cost) == D_OK &&
              unchanged == third && m.used == before && cost.clones == 0);
        CHECK(d_map_delete(&m, third, key(66, 0), &third, &cost) == D_OK &&
              third == 0 && cost.clones == 0);
    }
    for (unsigned operation = 0; operation < 3; ++operation)
    {
        for (unsigned cut = 1; cut <= 17; ++cut)
        {
            int mode = 1;
            struct dh_map m;
            unsigned root = 0;
            struct dh_metrics cost;
            CHECK(d_map_init(&m, (struct dh_dictionary){&mode, hash, eq}) ==
                  D_OK);
            CHECK(d_map_update(&m, 0, key(65, 0), 10, &root, NULL) == D_OK &&
                  d_map_update(&m, root, key(66, 0), 20, &root, NULL) == D_OK);
            unsigned before = m.used, out = 999;
            m.fail_at = cut;
            int status =
                operation == 2
                    ? d_map_delete(&m, root, key(65, 0), &out, &cost)
                    : d_map_update(&m, root, key(operation ? 65 : 67, 0), 99,
                                   &out, &cost);
            CHECK(status == D_FAILED && out == 999 && m.used == before &&
                  m.attempts == cut && cost.clones == cut - 1);
            CHECK(value(&m, root, key(65, 0)) == 10 &&
                  value(&m, root, key(66, 0)) == 20 &&
                  value(&m, root, key(67, 0)) == -999);
            invariant(&m, root, 0);
        }
    }
    int mode = 1;
    struct dh_map m;
    unsigned root = 0, roots[9] = {0};
    struct dh_metrics cost;
    CHECK(d_map_init(&m, (struct dh_dictionary){&mode, hash, eq}) == D_OK);
    for (unsigned n = 1; n <= 8; ++n)
    {
        CHECK(d_map_update(&m, root, key(65 + n - 1, 0), (int)n, &root,
                           &cost) == D_OK &&
              cost.clones == 17 && cost.comparisons == n - 1);
        roots[n] = root;
        CHECK(value(&m, roots[n - 1], key(65 + n - 1, 0)) == -999);
        int v = 999;
        CHECK(d_map_get(&m, root, key(90, 0), &v, &cost) == D_STALE &&
              v == 999 && cost.comparisons == n && cost.visited == 17);
    }
    unsigned before = m.used, out = 999;
    CHECK(d_map_update(&m, root, key(90, 0), 8, &out, &cost) == D_FULL &&
          out == 999 && m.used == before && m.attempts == 0 &&
          cost.comparisons == 8);
    invariant(&m, root, 0);
    unsigned successful = 0;
    while (m.used + 17 <= 512)
    {
        CHECK(d_map_update(&m, root, key(65, 0), 100, &root, NULL) == D_OK);
        ++successful;
    }
    CHECK(successful > 0);
    before = m.used;
    out = 999;
    CHECK(d_map_update(&m, root, key(65, 0), 200, &out, &cost) == D_FAILED &&
          out == 999 && m.used == before && value(&m, root, key(65, 0)) == 100);
    CHECK(value(&m, roots[8], key(65, 0)) == 1);
    CHECK(d_map_update(NULL, 0, key(0, 0), 0, &out, NULL) == D_INVALID);
    CHECK(d_map_get(&m, m.used + 1, key(0, 0), &mode, NULL) == D_INVALID);
}

static void observations(void)
{
    const struct dh_key keys[] = {{{65, 0}}, {{97, 0}}, {{66, 0}}, {{98, 0}},
                                  {{67, 1}}, {{99, 1}}, {{68, 1}}, {{100, 1}}};
    for (int mode = 0; mode < 2; ++mode)
    {
        for (unsigned a = 0; a < 8; ++a)
        {
            for (unsigned b = 0; b < 8; ++b)
            {
                struct dh_map m;
                unsigned roots[4] = {0};
                CHECK(d_map_init(&m, (struct dh_dictionary){&mode, hash, eq}) ==
                      D_OK);
                CHECK(d_map_update(&m, 0, keys[a], 10, &roots[1], NULL) ==
                      D_OK);
                CHECK(d_map_update(&m, roots[1], keys[b], 20, &roots[2],
                                   NULL) == D_OK);
                CHECK(d_map_delete(&m, roots[2], keys[a], &roots[3], NULL) ==
                      D_OK);
                for (unsigned version = 0; version < 4; ++version)
                {
                    for (unsigned q = 0; q < 8; ++q)
                    {
                        printf("M %d %u %u %u %u %d\n", mode, a, b, version, q,
                               value(&m, roots[version], keys[q]));
                    }
                }
            }
        }
    }
    int mode = 1;
    struct dh_map m;
    unsigned root = 0;
    struct dh_metrics cost;
    CHECK(d_map_init(&m, (struct dh_dictionary){&mode, hash, eq}) == D_OK);
    for (unsigned n = 1; n <= 9; ++n)
    {
        unsigned next = 999;
        int status =
            d_map_update(&m, root, key(64 + n, 0), (int)n, &next, &cost);
        printf("C %u %d %u %u %u %u\n", n, status, cost.comparisons,
               cost.clones, m.attempts, m.used);
        if (status == D_OK)
        {
            root = next;
        }
        else
        {
            CHECK(next == 999);
        }
    }
    printf("L %zu %zu %zu\n", sizeof(struct dh_map_node), sizeof(struct dh_map),
           sizeof(struct dh_key));
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "dictionary"))
    {
        checks();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        observations();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
