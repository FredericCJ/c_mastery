/* Explicit host integration: compile only with an authorized, separately named
 * algorithms implementation and its matching public header. Never auto-import. */
#include "systems_certificates.h"
#include "algo.h"
#include <stdio.h>
#include <string.h>

static int has(void *p, int32_t key)
{
    return algo_avl_contains((const algo_avl *)p, key);
}

static int put(void *p, int32_t key)
{
    return algo_avl_insert((algo_avl *)p, key);
}

static int snapshot(const algo_avl *source, struct sc_tree *out)
{
    unsigned i, j;
    struct sc_tree t = {0};
    t.root = source->root;
    t.count = (uint8_t)source->count;
    t.balanced = 1;
    for (i = 0; i < 64; i++)
    {
        const algo_node *n = &source->nodes[i];
        t.nodes[i] = (struct sc_node){
            n->key, n->left, n->right, -1, (uint8_t)n->height, n->live};
    }
    for (i = 0; i < 64; i++)
        if (t.nodes[i].live)
            for (j = 0; j < 2; j++)
            {
                int child = j ? t.nodes[i].right : t.nodes[i].left;
                if (child >= 0)
                {
                    if (child >= 64 || t.nodes[child].parent != -1)
                        return 1;
                    t.nodes[child].parent = (int16_t)i;
                }
            }
    *out = t;
    return 0;
}

int main(void)
{
    algo_avl tree, before;
    struct sc_tree cert;
    unsigned height, i;
    int32_t keys[] = {30, 10, 20, 50, 40, 60, 5, 15, 25, 35};
    algo_avl_init(&tree);
    for (i = 0; i < 10; i++)
    {
        if (sc_unique_insert(&tree, has, put, keys[i]) != SC_OK ||
            snapshot(&tree, &cert) || sc_tree_check(&cert, &height) != SC_OK)
            return 1;
    }
    before = tree;
    if (algo_avl_insert(&tree, 20) != ALGO_OK ||
        memcmp(&tree, &before, sizeof tree))
        return 1;
    if (sc_unique_insert(&tree, has, put, 20) != SC_DUPLICATE ||
        memcmp(&tree, &before, sizeof tree))
        return 1;
    for (i = 0; i < 10; i++)
    {
        if (algo_avl_delete(&tree, keys[i]) != ALGO_OK ||
            snapshot(&tree, &cert) || sc_tree_check(&cert, &height) != SC_OK)
            return 1;
    }
    if (height || tree.count)
        return 1;
    puts(
        "SC_INTEGRATION: baseline idempotence preserved; reject-duplicate adapter and20 mutation certificates passed");
    return 0;
}
