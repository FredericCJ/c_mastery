#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "CONTAINERS_INDEX_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

struct cd_alloc
{
    unsigned calls, fail, live;
};

static struct cd_bucket *take(void *v, unsigned n)
{
    struct cd_alloc *a = v;
    if (++a->calls == a->fail)
        return NULL;
    struct cd_bucket *p = malloc(n * sizeof *p);
    if (p)
        ++a->live;
    return p;
}

static void release(void *v, struct cd_bucket *p)
{
    struct cd_alloc *a = v;
    CHECK(a->live);
    --a->live;
    free(p);
}

static void hashes(void)
{
    struct cd_alloc a = {0};
    struct cd_hash h;
    struct cd_chain chain;
    cd_chain_init(&chain);
    CHECK(cd_hash_init(&h, 4, (struct cd_hash_allocator){&a, take, release},
                       0) == CD_OK);
    unsigned probes;
    int out = 99;
    for (unsigned k = 1; k <= 9; k += 4)
    {
        CHECK(cd_hash_put(&h, k, (int)k, &probes) == CD_OK &&
              cd_chain_put(&chain, k, (int)k) == CD_OK);
    }
    CHECK(h.live == 3 && h.occupied == 3 && cd_hash_delete(&h, 5) == CD_OK &&
          cd_chain_delete(&chain, 5) == CD_OK);
    for (unsigned k = 1; k <= 9; k += 8)
    {
        CHECK(cd_hash_get(&h, k, &out, &probes) == CD_OK && out == (int)k &&
              probes <= 4);
        CHECK(cd_chain_get(&chain, k, &out, &probes) == CD_OK &&
              out == (int)k && probes <= 3);
    }
    CHECK(h.live == 2 && h.occupied == 3 &&
          cd_hash_put(&h, 13, 13, &probes) == CD_OK && h.live == 3 &&
          h.occupied == 3);
    struct cd_bucket snapshot[4];
    memcpy(snapshot, h.slots, sizeof snapshot);
    struct cd_bucket *live = h.slots;
    a.fail = a.calls + 1;
    CHECK(cd_hash_rebuild(&h, 8) == CD_NOMEM && h.slots == live &&
          h.capacity == 4 && h.live == 3 && h.occupied == 3 &&
          !memcmp(snapshot, h.slots, sizeof snapshot) && a.live == 1);
    a.fail = 0;
    CHECK(cd_hash_rebuild(&h, 8) == CD_OK && h.live == 3 && h.occupied == 3 &&
          a.live == 1);
    CHECK(cd_hash_destroy(&h) == CD_OK && a.live == 0);
    CHECK(cd_hash_init(&h, 4, (struct cd_hash_allocator){&a, take, release},
                       0) == CD_OK);
    for (unsigned k = 0; k < 4; ++k)
        CHECK(cd_hash_put(&h, k, (int)k, &probes) == CD_OK);
    for (unsigned k = 0; k < 4; ++k)
        CHECK(cd_hash_delete(&h, k) == CD_OK);
    CHECK(h.live == 0 && h.occupied == 4 &&
          cd_hash_get(&h, 21, &out, &probes) == CD_MISSING && probes == 4);
    CHECK(cd_hash_put(&h, 9, 90, &probes) == CD_OK && probes == 4 &&
          h.occupied == 4);
    CHECK(cd_hash_rebuild(&h, 4) == CD_OK && h.occupied == 1 &&
          cd_hash_destroy(&h) == CD_OK);
    CHECK(cd_hash_init(&h, 16, (struct cd_hash_allocator){&a, take, release},
                       1) == CD_OK);
    for (unsigned k = 0; k < 16; ++k)
        CHECK(cd_hash_put(&h, k, (int)k, &probes) == CD_OK && probes == k + 1);
    CHECK(cd_hash_get(&h, 99, &out, &probes) == CD_MISSING && probes == 16);
    CHECK(cd_hash_destroy(&h) == CD_OK);
    struct cd_byte_map m = {0};
    unsigned char buffer[2] = {'A', 0};
    CHECK(cd_bytemap_put(&m, buffer, 2, 7) == CD_OK);
    buffer[0] = 'B';
    unsigned char original[2] = {'A', 0};
    CHECK(cd_bytemap_get(&m, original, 2, &out) == CD_OK && out == 7 &&
          cd_bytemap_get(&m, buffer, 2, &out) == CD_MISSING);
    /* Defined borrowed-key counterexample: stored placement uses old hash; live bytes changed. */
    unsigned old_bucket = original[0] % 4, new_bucket = buffer[0] % 4;
    CHECK(old_bucket != new_bucket);
    /* Case-insensitive equality with case-sensitive mod-4 hash is inconsistent. */
    CHECK(('A' | 32) ==
          ('a' | 32)); /* use mod-3: modulo4 happens to preserve this pair */
    CHECK((unsigned)'A' % 3 != (unsigned)'a' % 3);
    puts(
        "CONTAINERS_HASH_PASS tombstone all_tombstones transactional_rebuild clustered copied_key inconsistent_callbacks");
}

static void tries(void)
{
    struct cd_trie t;
    cd_trie_init(&t);
    unsigned char a[3] = {65, 0, 66};
    CHECK(cd_trie_insert(&t, NULL, 0, 0) == CD_OK);
    for (size_t n = 1; n <= 3; ++n)
        CHECK(cd_trie_insert(&t, a, n, 0) == CD_OK);
    struct cd_trie_cursor cursor;
    CHECK(cd_trie_prefix(&t, a, 1, &cursor) == CD_OK && cursor.count == 3);
    struct cd_key key;
    unsigned seen = 0;
    while (cd_trie_next(&cursor, &key) == CD_OK)
    {
        CHECK(key.length >= 1 && key.length <= 3 &&
              !memcmp(key.bytes, a, key.length));
        seen |= 1u << key.length;
    }
    CHECK(seen == 14);
    unsigned before = t.count;
    CHECK(cd_trie_delete(&t, a, 1) == CD_OK && t.count == before &&
          cd_trie_prefix(&t, a, 1, &cursor) == CD_OK && cursor.count == 2);
    CHECK(cd_trie_delete(&t, a, 3) == CD_OK && t.count == before - 1);
    struct cd_trie saved = t;
    unsigned char other[3] = {70, 71, 72};
    for (unsigned fail = 1; fail <= 3; ++fail)
    {
        CHECK(cd_trie_insert(&t, other, 3, fail) == CD_NOMEM &&
              !memcmp(&t, &saved, sizeof t));
    }
    CHECK(cd_trie_prefix(&t, NULL, 0, &cursor) == CD_OK);
    CHECK(cd_trie_insert(&t, other, 3, 0) == CD_OK &&
          cd_trie_next(&cursor, &key) == CD_BUSY);
    unsigned char parser[3] = {'x', 'y', 'z'};
    CHECK(cd_trie_insert(&t, parser, 3, 0) == CD_OK);
    memset(parser, 'q', 3);
    CHECK(cd_trie_prefix(&t, (const unsigned char *)"xyz", 3, &cursor) ==
              CD_OK &&
          cursor.count == 1);
    cd_trie_init(&t);
    CHECK(cd_trie_insert(&t, (const unsigned char *)"abcdef", 6, 0) == CD_OK &&
          cd_trie_insert(&t, (const unsigned char *)"abcxyz", 6, 0) == CD_OK &&
          t.count == 10);
    printf(
        "CONTAINERS_TRIE_PASS nodes=%u node_bytes=%zu allocated_node_array=%zu\n",
        t.count, t.count * sizeof(struct cd_trie_node), sizeof t.nodes);
}

static void sets(void)
{
    uint32_t join, meet, difference, mask;
    CHECK(cd_bitset_algebra((1u << 1) | (1u << 2) | (1u << 5),
                            (1u << 2) | (1u << 3), 10, &join, &meet,
                            &difference) == CD_OK &&
          join == 46 && meet == 4 && difference == 34);
    CHECK(cd_bitset_algebra(UINT32_MAX, 0, 10, &join, &meet, &difference) ==
              CD_OK &&
          join == 1023);
    CHECK(cd_bitset_mask(32, &mask) == CD_OK && mask == UINT32_MAX &&
          cd_bitset_mask(0, &mask) == CD_OK && !mask);
    unsigned position = 0, key;
    CHECK(cd_bitset_next(34, 10, &position, &key) == CD_OK && key == 1 &&
          cd_bitset_next(34, 10, &position, &key) == CD_OK && key == 5 &&
          cd_bitset_next(34, 10, &position, &key) == CD_EMPTY &&
          position == 10);
    for (unsigned universe = 0; universe <= 10; ++universe)
    {
        uint32_t top = UINT32_C(1) << universe;
        for (uint32_t a = 0; a < top; ++a)
        {
            CHECK(cd_bitset_algebra(a, ~a, universe, &join, &meet,
                                    &difference) == CD_OK &&
                  join == top - 1 && !meet && difference == a);
        }
    }
    struct cd_sparse s;
    cd_sparse_init(&s);
    CHECK(cd_sparse_insert(&s, 2) == CD_OK &&
          cd_sparse_insert(&s, 5) == CD_OK &&
          cd_sparse_remove(&s, 2) == CD_OK && cd_sparse_has(&s, 5) &&
          !cd_sparse_has(&s, 2));
    for (unsigned round = 0; round < 12; ++round)
    {
        CHECK(cd_sparse_insert(&s, 17) == CD_OK);
        unsigned oldgeneration = s.generation;
        cd_sparse_clear(&s);
        CHECK(!cd_sparse_has(&s, 17) && !cd_sparse_has(&s, 5) &&
              s.generation != oldgeneration);
    }
    struct cd_sets u;
    CHECK(cd_sets_init(&u, 6) == CD_OK);
    size_t empty = u.used;
    CHECK(cd_sets_union(&u, 0, 2) == CD_OK &&
          cd_sets_union(&u, 3, 4) == CD_OK && cd_sets_union(&u, 2, 4) == CD_OK);
    unsigned r[6];
    for (unsigned i = 0; i < 6; ++i)
        CHECK(cd_sets_find(&u, i, &r[i]) == CD_OK);
    CHECK(r[0] == r[2] && r[2] == r[3] && r[3] == r[4] && r[1] != r[0] &&
          r[5] != r[0] && r[1] != r[5] && u.size[r[0]] == 4);
    CHECK(cd_sets_union(&u, 0, 4) == CD_OK && u.size[r[0]] == 4);
    CHECK(cd_sets_rollback(&u, empty) == CD_OK);
    for (unsigned i = 0; i < 6; ++i)
        CHECK(u.parent[i] == i && u.size[i] == 1);
    CHECK(cd_sets_union(&u, 0, 1) == CD_OK &&
          cd_sets_union(&u, 2, 3) == CD_OK && cd_sets_union(&u, 0, 2) == CD_OK);
    CHECK(u.parent[3] == 2);
    size_t before = u.used;
    CHECK(cd_sets_find(&u, 3, &key) == CD_OK && key == 0 && u.parent[3] == 0 &&
          u.used == before + 1);
    CHECK(cd_sets_rollback(&u, before) == CD_OK && u.parent[3] == 2);
    /* A declared finite log can refuse another update without changing the partition. */
    u.used = 128;
    struct cd_sets old = u;
    CHECK(cd_sets_find(&u, 3, &key) == CD_FULL && !memcmp(&old, &u, sizeof u));
    puts(
        "CONTAINERS_SETS_PASS bitset_exhaustive_complements two_bit_wrap rollback_compression");
}

static void graphs(void)
{
    struct cd_edge edges[3] = {{0, 1, 4}, {0, 2, 5}, {2, 1, 6}};
    struct cd_csr csr;
    CHECK(cd_csr_build(&csr, 4, edges, 3, 0) == CD_OK);
    unsigned expected[5] = {0, 2, 2, 3, 3};
    for (unsigned i = 0; i <= 4; ++i)
        CHECK(csr.offset[i] == expected[i]);
    unsigned matrix[4][4] = {{0}}, list[4][4] = {{0}}, degrees[4] = {0};
    for (unsigned i = 0; i < 3; ++i)
    {
        matrix[edges[i].from][edges[i].to] = 1;
        list[edges[i].from][degrees[edges[i].from]++] = edges[i].to;
    }
    for (unsigned v = 0; v < 4; ++v)
    {
        CHECK(csr.offset[v] <= csr.offset[v + 1] &&
              csr.offset[v + 1] <= csr.count &&
              degrees[v] == csr.offset[v + 1] - csr.offset[v]);
        for (unsigned j = 0; j < degrees[v]; ++j)
            CHECK(csr.edges[csr.offset[v] + j].to == list[v][j] &&
                  matrix[v][list[v][j]]);
    }
    struct cd_csr before = csr;
    struct cd_edge bad = {4, 0, 0};
    CHECK(cd_csr_build(&csr, 4, &bad, 1, 0) == CD_INVALID &&
          !memcmp(&before, &csr, sizeof csr));
    struct cd_edge parallel[2] = {{0, 1, 4}, {0, 1, 9}};
    CHECK(cd_csr_build(&csr, 4, parallel, 2, 0) == CD_DUPLICATE &&
          cd_csr_build(&csr, 4, parallel, 2, 1) == CD_OK && csr.count == 2 &&
          csr.edges[0].weight == 4 && csr.edges[1].weight == 9);
    CHECK(matrix[0][1] ==
          1); /* Boolean projection cannot recover multiplicity/weights. */
    puts(
        "CONTAINERS_GRAPH_PASS list_matrix_CSR isolated_vertex parallel_weight_loss");
}

static void filters(void)
{
    unsigned char filter = 0;
    CHECK(cd_filter_insert(&filter, 1) == CD_OK &&
          cd_filter_insert(&filter, 2) == CD_OK);
    unsigned matches = 0, false_hits = 0;
    for (unsigned k = 0; k < 16; ++k)
    {
        unsigned result;
        CHECK(cd_filter_query(filter, k, &result) == CD_OK);
        if (k == 1 || k == 2)
            CHECK(result);
        if (result)
        {
            ++matches;
            if (k != 1 && k != 2)
                ++false_hits;
        }
    }
    CHECK(matches == 4 && false_hits == 2);
    unsigned char before = filter;
    CHECK(cd_filter_insert(&filter, 16) == CD_INVALID && filter == before);
    for (unsigned k = 0; k < 16; ++k)
    {
        CHECK(cd_filter_insert(&filter, k) == CD_OK);
    }
    CHECK(filter == 255);
    for (unsigned k = 0; k < 16; ++k)
    {
        unsigned result;
        CHECK(cd_filter_query(filter, k, &result) == CD_OK && result);
    }
    filter = 0;
    CHECK(cd_filter_insert(&filter, 1) == CD_OK &&
          cd_filter_insert(&filter, 9) == CD_OK);
    filter &= (unsigned char)~((1u << 1) | (1u << 4));
    unsigned result;
    CHECK(
        cd_filter_query(filter, 9, &result) == CD_OK &&
        !result); /* known-present key9 was lost by clearing key1's shared bits */
    struct cd_counting_filter counting = {0};
    for (unsigned i = 0; i < 3; ++i)
        CHECK(cd_counting_insert(&counting, 1) == CD_OK);
    struct cd_counting_filter saved = counting;
    CHECK(cd_counting_insert(&counting, 9) == CD_FULL &&
          !memcmp(&saved, &counting, sizeof saved));
    CHECK(cd_counting_delete(&counting, 9) == CD_MISSING &&
          !memcmp(&saved, &counting, sizeof saved));
    CHECK(cd_counting_delete(&counting, 1) == CD_OK &&
          cd_counting_insert(&counting, 9) == CD_OK && counting.cells[1] == 3 &&
          counting.cells[4] == 3);
    printf(
        "CONTAINERS_FILTER_PASS matches=%u false_positives=%u saturation_and_counter_admission\n",
        matches, false_hits);
}

int main(void)
{
    hashes();
    tries();
    sets();
    graphs();
    filters();
    return 0;
}
