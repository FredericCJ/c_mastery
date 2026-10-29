#include "dod_rope_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "rope_stats";
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "DOD_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static unsigned chain(struct d_rope *r, unsigned n)
{
    unsigned root = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned leaf;
        unsigned char value = (unsigned char)i;
        CHECK(d_rope_leaf(r, &value, 1, &leaf) == D_OK &&
              d_rope_concat(r, root, leaf, &root) == D_OK);
    }
    return root;
}

static void checks(void)
{
    struct d_rope r;
    d_rope_init(&r);
    unsigned root = chain(&r, 16), balanced = 0;
    struct d_rope_stats before, after;
    CHECK(d_rope_stats(&r, root, &before) == D_OK && before.depth == 15 &&
          !before.balanced && before.unique_leaves == 16 &&
          before.stored_bytes == 16 && before.leaf_slack == 112 &&
          before.reachable_nodes == 31 && before.fibonacci_min == 129);
    CHECK(d_rope_balance(&r, root, &balanced) == D_OK);
    CHECK(d_rope_stats(&r, balanced, &after) == D_OK && after.depth == 1 &&
          after.balanced && after.reachable_nodes == 3 &&
          after.retained_unreachable == 31 && after.leaf_slack == 0);
    unsigned char a[128], b[128];
    size_t an, bn;
    CHECK(d_rope_flatten(&r, root, a, 128, &an) == D_OK &&
          d_rope_flatten(&r, balanced, b, 128, &bn) == D_OK && an == bn &&
          !memcmp(a, b, an));
    d_rope_init(&r);
    unsigned leaf;
    CHECK(d_rope_leaf(&r, (const unsigned char *)"abc", 3, &leaf) == D_OK &&
          d_rope_concat(&r, leaf, leaf, &root) == D_OK);
    CHECK(d_rope_stats(&r, root, &after) == D_OK && after.logical_bytes == 6 &&
          after.stored_bytes == 3 && after.unique_leaves == 1 &&
          after.reachable_nodes == 2);
    /* Root-only Fibonacci criterion is not recursive subtree balance. */
    d_rope_init(&r);
    unsigned left = chain(&r, 6);
    CHECK(d_rope_stats(&r, left, &before) == D_OK && !before.balanced);
    unsigned char bytes[8] = {0};
    unsigned nodes[8];
    for (unsigned i = 0; i < 8; ++i)
        CHECK(d_rope_leaf(&r, bytes, 8, &nodes[i]) == D_OK);
    for (unsigned width = 8; width > 1; width /= 2)
        for (unsigned i = 0; i < width / 2; ++i)
            CHECK(d_rope_concat(&r, nodes[2 * i], nodes[2 * i + 1],
                                &nodes[i]) == D_OK);
    CHECK(d_rope_concat(&r, left, nodes[0], &root) == D_OK &&
          d_rope_stats(&r, root, &after) == D_OK && after.depth == 6 &&
          after.logical_bytes == 70 && after.fibonacci_min == 21 &&
          after.balanced);
    /* Snapshot compaction uses exactly16+48 additional backing bytes. */
    unsigned char original[16], added[128], fresh[16], extra[48];
    for (unsigned i = 0; i < 16; ++i)
        original[i] = (unsigned char)i;
    struct d_piece_table old;
    CHECK(d_piece_init(&old, original, 16, added, 128) == D_OK);
    for (unsigned i = 0; i < 12; ++i)
    {
        unsigned char c = 99;
        CHECK(d_piece_insert(&old, 8, &c, 1) == D_OK &&
              d_piece_delete(&old, 8, 1) == D_OK);
    }
    CHECK(old.length == 16 && old.add_length == 12);
    struct d_piece_table snapshot = old, next = {0};
    for (unsigned i = 0; i < 16; ++i)
        fresh[i] = 200;
    CHECK(d_piece_compact(&old, fresh, 15, extra, 48, &next) == D_FULL &&
          next.original == NULL);
    for (unsigned i = 0; i < 16; ++i)
        CHECK(fresh[i] == 200);
    CHECK(d_piece_compact(&old, fresh, 16, extra, 48, &next) == D_OK &&
          next.add_length == 0 && next.count == 1);
    for (unsigned i = 0; i < 48; ++i)
        extra[i] = 50;
    unsigned char insertion[48];
    for (unsigned i = 0; i < 48; ++i)
        insertion[i] = 50;
    CHECK(d_piece_insert(&next, 16, insertion, 48) == D_OK &&
          d_piece_insert(&next, 0, insertion, 1) == D_FULL);
    CHECK(d_piece_flatten(&snapshot, a, 128, &an) == D_OK && an == 16 &&
          !memcmp(a, original, 16));
    /* All old descriptors are retired before caller reuses their backing. */
    old = (struct d_piece_table){0};
    snapshot = (struct d_piece_table){0};
    for (unsigned i = 0; i < 16; ++i)
        original[i] = 255;
    for (unsigned i = 0; i < 128; ++i)
        added[i] = 255;
    CHECK(d_piece_flatten(&next, a, 128, &an) == D_OK && an == 64);
    for (unsigned i = 0; i < 16; ++i)
        CHECK(a[i] == i);
    for (unsigned i = 16; i < 64; ++i)
        CHECK(a[i] == 50);
}

static void observations(void)
{
    for (unsigned n = 1; n <= 16; ++n)
    {
        struct d_rope r;
        d_rope_init(&r);
        unsigned root = chain(&r, n), newroot;
        struct d_rope_stats s;
        CHECK(d_rope_stats(&r, root, &s) == D_OK);
        printf("P %u 0 %u %u %u %u %u %u %u %u\n", n, s.depth, s.logical_bytes,
               s.reachable_nodes, s.unique_leaves, s.stored_bytes, s.leaf_slack,
               s.fibonacci_min, s.balanced);
        CHECK(d_rope_balance(&r, root, &newroot) == D_OK &&
              d_rope_stats(&r, newroot, &s) == D_OK);
        printf("P %u 1 %u %u %u %u %u %u %u %u\n", n, s.depth, s.logical_bytes,
               s.reachable_nodes, s.unique_leaves, s.stored_bytes, s.leaf_slack,
               s.fibonacci_min, s.balanced);
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "rope_stats"))
        checks();
    else if (!strcmp(argv[1], "oracle"))
        observations();
    else
        CHECK(0);
    return 0;
}
