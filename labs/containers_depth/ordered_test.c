#include "containers.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "CONTAINERS_ORDER_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static int verify_node(const struct cd_tree *t, int at, int parent, int64_t low,
                       int64_t high, uint32_t *seen)
{
    if (at < 0)
    {
        return 0;
    }
    CHECK(at < 32 && !(*seen & (UINT32_C(1) << (unsigned)at)));
    *seen |= UINT32_C(1) << (unsigned)at;
    const struct cd_tree_node *n = &t->nodes[at];
    CHECK(n->live && n->parent == parent && n->key > low && n->key < high &&
          n->multiplicity);
    int left = verify_node(t, n->left, at, low, n->key, seen),
        right = verify_node(t, n->right, at, n->key, high, seen);
    CHECK(n->height == 1 + (left > right ? left : right));
    if (t->balanced)
    {
        CHECK(left - right <= 1 && right - left <= 1);
    }
    return n->height;
}

static void verify(const struct cd_tree *t)
{
    uint32_t seen = 0;
    verify_node(t, t->root, -1, INT64_MIN, INT64_MAX, &seen);
    unsigned count = 0;
    for (unsigned i = 0; i < 32; ++i)
    {
        CHECK((t->nodes[i].live != 0) == ((seen & (UINT32_C(1) << i)) != 0));
        count += t->nodes[i].live != 0;
    }
    CHECK(count == t->count);
}

static unsigned histories;

static void permutation(int *a, unsigned at)
{
    if (at == 6)
    {
        for (unsigned variant = 0; variant < 2; ++variant)
        {
            struct cd_tree t;
            cd_tree_init(&t, 1, 0);
            int slot = 0;
            for (unsigned i = 0; i < 6; ++i)
            {
                CHECK(cd_tree_insert(&t, variant ? (int)i : a[i], 0, &slot) ==
                      CD_OK);
                verify(&t);
            }
            for (unsigned i = 0; i < 6; ++i)
            {
                CHECK(cd_tree_delete(&t, variant ? a[i] : (int)i) == CD_OK);
                verify(&t);
            }
            CHECK(t.root == -1 && t.count == 0);
            ++histories;
        }
        return;
    }
    for (unsigned i = at; i < 6; ++i)
    {
        int x = a[at];
        a[at] = a[i];
        a[i] = x;
        permutation(a, at + 1);
        x = a[at];
        a[at] = a[i];
        a[i] = x;
    }
}

int main(void)
{
    struct cd_list list;
    cd_list_init(&list);
    struct cd_link a = {0, 0, 0, 1}, b = {0, 0, 0, 2}, c = {0, 0, 0, 3};
    CHECK(cd_list_push(&list, &a, 0) == CD_OK &&
          cd_list_push(&list, &b, 0) == CD_OK &&
          cd_list_push(&list, &c, 0) == CD_OK);
    CHECK(cd_list_remove(&list, &b) == CD_OK && a.next == &c &&
          c.previous == &a && b.owner == NULL &&
          cd_list_remove(&list, &b) == CD_INVALID);
    struct cd_link *at = list.head;
    while (at)
    {
        struct cd_link *next = at->next;
        CHECK(cd_list_remove(&list, at) == CD_OK);
        at = next;
    }
    CHECK(!list.head && !list.tail && !list.count);
    struct cd_single sa = {0, 1, 0}, sb = {0, 2, 0}, sc = {0, 3, 0};
    struct cd_single_list singly = {0};
    CHECK(cd_single_append(&singly, &sa) == CD_OK &&
          cd_single_append(&singly, &sb) == CD_OK &&
          cd_single_append(&singly, &sc) == CD_OK);
    CHECK(cd_single_remove(&singly, &sb) == CD_OK && sa.next == &sc);
    CHECK(cd_single_remove(&singly, &sa) == CD_OK && singly.head == &sc &&
          cd_single_remove(&singly, &sc) == CD_OK && !singly.tail);
    struct cd_circle first, second;
    cd_circle_init(&first);
    cd_circle_init(&second);

    struct
    {
        int value;
        struct cd_link links[2];
    } task = {7, {{0}, {0}}};

    CHECK(cd_circle_append(&first, &task.links[0]) == CD_OK &&
          cd_circle_append(&second, &task.links[1]) == CD_OK);
    CHECK(cd_circle_append(&second, &task.links[0]) == CD_INVALID);
    CHECK(cd_circle_remove(&first, &task.links[0]) == CD_OK &&
          task.links[1].owner == &second.identity);
    CHECK(cd_circle_remove(&second, &task.links[1]) == CD_OK &&
          first.sentinel.next == &first.sentinel &&
          second.sentinel.previous == &second.sentinel);
    int keys[5] = {7, 1, 7, 0, -2};
    struct cd_heap heap;
    struct cd_heap_handle handles[16];
    CHECK(cd_heap_build(&heap, keys, 5, 0) == CD_OK);
    uint64_t bottom = heap.comparisons;
    cd_heap_init(&heap, 0);
    for (unsigned i = 0; i < 5; ++i)
    {
        CHECK(cd_heap_insert(&heap, keys[i], i, &handles[i]) == CD_OK);
    }
    uint64_t repeated = heap.comparisons;
    int value;
    unsigned id;
    for (unsigned i = 0; i < 5; ++i)
    {
        CHECK(cd_heap_get(&heap, handles[i], &value, &id) == CD_OK &&
              value == keys[i] && id == i);
    }
    CHECK(cd_heap_insert(&heap, 3, 5, &handles[5]) == CD_OK);
    CHECK(cd_heap_change(&heap, handles[0], -3) == CD_OK &&
          cd_heap_take(&heap, &value, &id) == CD_OK && value == -3 && id == 0 &&
          cd_heap_get(&heap, handles[0], &value, &id) == CD_INVALID);
    int expected[5] = {-2, 0, 1, 3, 7};
    for (unsigned i = 0; i < 5; ++i)
    {
        CHECK(cd_heap_take(&heap, &value, &id) == CD_OK &&
              value == expected[i]);
    }
    CHECK(cd_heap_take(&heap, &value, &id) == CD_EMPTY);
    cd_heap_init(&heap, 1);
    CHECK(cd_heap_insert(&heap, 4, 90, &handles[0]) == CD_OK &&
          cd_heap_insert(&heap, 4, 91, &handles[1]) == CD_OK);
    CHECK(cd_heap_take(&heap, &value, &id) == CD_OK && id == 90 &&
          cd_heap_take(&heap, &value, &id) == CD_OK && id == 91);
    cd_heap_init(&heap, 0);
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        CHECK(cd_heap_insert(&heap, 1, iteration, &handles[iteration]) ==
                  CD_OK &&
              handles[iteration].slot == 0 &&
              cd_heap_take(&heap, &value, &id) == CD_OK);
    }
    CHECK(heap.state[0] == 2 &&
          cd_heap_insert(&heap, 1, 4, &handles[3]) == CD_OK &&
          handles[3].slot == 1);
    struct cd_tree tree;
    cd_tree_init(&tree, 0, 0);
    int order[7] = {4, 2, 6, 1, 3, 5, 7}, slot;
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(cd_tree_insert(&tree, order[i], 0, &slot) == CD_OK);
    }
    verify(&tree);
    int old_root = cd_tree_find(&tree, 4), successor = cd_tree_find(&tree, 5);
    struct cd_tree_node *identity = &tree.nodes[successor];
    CHECK(cd_tree_delete(&tree, 4) == CD_OK && !tree.nodes[old_root].live &&
          identity == &tree.nodes[successor] && identity->key == 5 &&
          identity->live && tree.root == successor);
    verify(&tree);
    struct cd_tree snapshot = tree;
    CHECK(cd_tree_insert(&tree, 20, 1, &slot) == CD_NOMEM &&
          !memcmp(&snapshot, &tree, sizeof tree));
    for (unsigned shape = 0; shape < 3; ++shape)
    {
        cd_tree_init(&tree, 0, 0);
        for (unsigned i = 0; i < 7; ++i)
        {
            CHECK(cd_tree_insert(&tree, order[i], 0, &slot) == CD_OK);
        }
        if (shape == 1)
        {
            CHECK(cd_tree_delete(&tree, 1) == CD_OK);
            verify(&tree);
        }
        CHECK(cd_tree_delete(&tree, shape == 0   ? 1
                                    : shape == 1 ? 2
                                                 : 4) == CD_OK);
        verify(&tree);
    }
    cd_tree_init(&tree, 0, 1);
    CHECK(cd_tree_insert(&tree, 3, 0, &slot) == CD_OK &&
          cd_tree_insert(&tree, 3, 0, &slot) == CD_OK &&
          tree.nodes[slot].multiplicity == 2 &&
          cd_tree_delete(&tree, 3) == CD_OK &&
          tree.nodes[slot].multiplicity == 1);
    verify(&tree);
    cd_tree_init(&tree, 0, 0);
    for (int i = 7; i >= 0; --i)
    {
        CHECK(cd_tree_insert(&tree, i, 0, &slot) == CD_OK);
    }
    unsigned stack[8];
    int output[10];
    size_t used = 99;
    CHECK(cd_tree_walk(&tree, stack, 2, output, 10, &used) == CD_FULL &&
          used == 0);
    CHECK(cd_tree_walk(&tree, stack, 8, output, 10, &used) == CD_OK &&
          used == 8);
    for (unsigned i = 0; i < 8; ++i)
    {
        CHECK(output[i] == (int)i);
    }
    CHECK(cd_tree_parent_walk(&tree, output, 4, &used) == CD_FULL && used == 4);
    CHECK(cd_tree_parent_walk(&tree, output, 10, &used) == CD_OK && used == 8);
    int rotations[4][3] = {
        {30, 20, 10}, {10, 20, 30}, {30, 10, 20}, {10, 30, 20}};
    for (unsigned kind = 0; kind < 4; ++kind)
    {
        cd_tree_init(&tree, 1, 0);
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(cd_tree_insert(&tree, rotations[kind][i], 0, &slot) == CD_OK);
        }
        verify(&tree);
        CHECK(tree.nodes[tree.root].key == 20 &&
              tree.nodes[tree.root].height == 2 &&
              tree.rotations == (kind < 2 ? 1u : 2u));
        if (kind == 0)
        {
            struct cd_tree old = tree;
            CHECK(cd_tree_insert(&tree, 20, 0, &slot) == CD_DUPLICATE &&
                  !memcmp(&old, &tree, sizeof tree));
            CHECK(cd_tree_delete(&tree, 30) == CD_OK);
            verify(&tree);
            CHECK(tree.nodes[tree.root].height == 2);
        }
    }
    int permutation_values[6] = {0, 1, 2, 3, 4, 5};
    permutation(permutation_values, 0);
    CHECK(histories == 1440);
    for (unsigned round = 0; round < 32; ++round)
    {
        cd_tree_init(&tree, 1, 0);
        for (unsigned i = 0; i < 32; ++i)
        {
            CHECK(cd_tree_insert(&tree, (int)((i * 13 + round) % 32), 0,
                                 &slot) == CD_OK);
            verify(&tree);
        }
        for (unsigned i = 0; i < 32; ++i)
        {
            CHECK(cd_tree_delete(&tree, (int)((i * 19 + round) % 32)) == CD_OK);
            verify(&tree);
        }
    }
    unsigned cascade = 0;
    for (unsigned round = 0; round < 32 && !cascade; ++round)
    {
        cd_tree_init(&tree, 1, 0);
        for (unsigned i = 0; i < 32; ++i)
        {
            CHECK(cd_tree_insert(&tree, (int)((i * 13 + round) % 32), 0,
                                 &slot) == CD_OK);
        }
        for (unsigned i = 0; i < 32; ++i)
        {
            unsigned before = tree.repair_steps, rot = tree.rotations;
            int removed = (int)((i * 19 + round) % 32);
            CHECK(cd_tree_delete(&tree, removed) == CD_OK);
            verify(&tree);
            if (tree.repair_steps - before >= 3 && tree.rotations - rot >= 2)
            {
                printf(
                    "CONTAINERS_CASCADE round=%u delete_position=%u key=%d ancestors=%u rotations=%u\n",
                    round, i, removed, tree.repair_steps - before,
                    tree.rotations - rot);
                cascade = 1;
                break;
            }
        }
    }
    CHECK(cascade);
    printf(
        "CONTAINERS_ORDER_PASS histories=%u bottom_up_comparisons=%llu insertion_comparisons=%llu\n",
        histories, (unsigned long long)bottom, (unsigned long long)repeated);
    return 0;
}
