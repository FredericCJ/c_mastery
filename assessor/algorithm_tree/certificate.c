#include "tree.h"
#include <limits.h>
struct at_work *at_work;
struct at_pool *at_pool;

void at_init(struct at_pool *p)
{
    algo_avl_init(&p->tree);
    p->free_count = 64;
    p->allocation_mode = UINT_MAX;
    for (unsigned i = 0; i < 64; ++i)
    {
        p->free_slots[i] = 63 - i;
        p->generation[i] = 1;
    }
}

static void begin(struct at_pool *p, struct at_work *w)
{
    at_pool = p;
    at_work = w;
    if (w)
    {
        *w = (struct at_work){0};
    }
}

static void end(void)
{
    at_work = NULL;
    at_pool = NULL;
}

int at_contains(struct at_pool *p, int32_t key, unsigned free_list,
                struct at_work *w)
{
    if (!p || free_list > 1 ||
        (p->allocation_mode != UINT_MAX && p->allocation_mode != free_list))
    {
        return -1;
    }
    begin(p, w);
    int found = free_list ? at_free_contains(&p->tree, key)
                          : at_observed_contains(&p->tree, key);
    end();
    return found;
}

int at_insert(struct at_pool *p, int32_t key, unsigned free_list,
              struct at_work *w)
{
    if (!p || free_list > 1 ||
        (p->allocation_mode != UINT_MAX && p->allocation_mode != free_list))
    {
        return ALGO_BAD;
    }
    begin(p, w);
    int duplicate = free_list ? at_free_contains(&p->tree, key)
                              : at_observed_contains(&p->tree, key);
#ifdef AT_MUTANT_DUPLICATE
    if (duplicate)
    {
        end();
        return ALGO_OK;
    }
#else
    if (duplicate)
    {
        end();
        return AT_DUPLICATE;
    }
#endif
    int status = free_list ? at_free_insert(&p->tree, key)
                           : at_observed_insert(&p->tree, key);
    end();
    if (status == ALGO_OK)
    {
        p->allocation_mode = free_list;
    }
    return status;
}

int at_delete(struct at_pool *p, int32_t key, unsigned free_list,
              struct at_work *w)
{
    if (!p || free_list > 1 ||
        (p->allocation_mode != UINT_MAX && p->allocation_mode != free_list))
    {
        return ALGO_BAD;
    }
    unsigned live[64];
    for (unsigned i = 0; i < 64; ++i)
    {
        live[i] = p->tree.nodes[i].live;
    }
    begin(p, w);
    int status = free_list ? at_free_delete(&p->tree, key)
                           : at_observed_delete(&p->tree, key);
    end();
    if (status == ALGO_OK)
    {
        for (unsigned i = 0; i < 64; ++i)
        {
            if (live[i] && !p->tree.nodes[i].live)
            {
                ++p->generation[i];
            }
        }
    }
    return status;
}

int at_certificate(const struct at_pool *p, struct at_certificate *out)
{
    if (!p || !out)
    {
        return ALGO_BAD;
    }
    struct at_certificate c = {0};
    c.root = p->tree.root;
    c.capacity = 64;
    c.count = p->tree.count;
    for (unsigned i = 0; i < 64; ++i)
    {
        algo_node n = p->tree.nodes[i];
        c.nodes[i] = (struct at_node){
            n.key, n.left, n.right, -1, n.height, n.live, 0, p->generation[i]};
    }
    for (unsigned i = 0; i < 64; ++i)
    {
        if (c.nodes[i].live)
        {
            int child[2] = {c.nodes[i].left, c.nodes[i].right};
            for (unsigned j = 0; j < 2; ++j)
            {
                if (child[j] >= 0)
                {
                    if (child[j] >= 64)
                    {
                        return ALGO_BAD;
                    }
                    c.nodes[child[j]].parent = (int)i;
                }
            }
        }
    }
    *out = c;
    return ALGO_OK;
}

int at_cd_certificate(const struct cd_tree *p, struct at_certificate *out)
{
    if (!p || !out)
    {
        return ALGO_BAD;
    }
    struct at_certificate c = {0};
    c.root = p->root;
    c.capacity = 32;
    c.count = p->count;
    for (unsigned i = 0; i < 32; ++i)
    {
        struct cd_tree_node n = p->nodes[i];
        c.nodes[i] = (struct at_node){n.key,    n.left, n.right, n.parent,
                                      n.height, n.live, 0,       n.generation};
    }
    *out = c;
    return ALGO_OK;
}

struct at_check
{
    const struct at_certificate *c;
    unsigned seen[64], policy;
    int failed;
};

static int visit(struct at_check *x, int at, int parent, int64_t low,
                 int64_t high, int *black)
{
    if (at == -1)
    {
        *black = 1;
        return 0;
    }
    if (at < 0 || (unsigned)at >= x->c->capacity || x->seen[at] ||
        !x->c->nodes[at].live)
    {
        x->failed = 1;
        return 0;
    }
    x->seen[at] = 1;
    struct at_node n = x->c->nodes[at];
    if (n.parent != parent || n.key <= low || n.key >= high || n.red > 1)
    {
        x->failed = 1;
        return 0;
    }
    int lb = 0, rb = 0;
    int l = visit(x, n.left, at, low, n.key, &lb),
        r = visit(x, n.right, at, n.key, high, &rb);
    int h = 1 + (l > r ? l : r);
#ifndef AT_MUTANT_HEIGHT
    if (x->policy != 2 && n.height != h)
    {
        x->failed = 1;
    }
#endif
    if (x->policy == 1 && (l - r > 1 || r - l > 1))
    {
        x->failed = 1;
    }
    if (x->policy == 2)
    {
        if (lb != rb ||
            (n.red && ((n.left >= 0 && n.left < (int)x->c->capacity &&
                        x->c->nodes[n.left].red) ||
                       (n.right >= 0 && n.right < (int)x->c->capacity &&
                        x->c->nodes[n.right].red))))
        {
            x->failed = 1;
        }
        *black = lb + !n.red;
    }
    else
    {
        *black = 0;
    }
    return h;
}

int at_check(const struct at_certificate *c, unsigned policy, int *out)
{
    if (!c || !out || !c->capacity || c->capacity > 64 ||
        c->count > c->capacity || policy > 2)
    {
        return ALGO_BAD;
    }
    struct at_check x = {.c = c, .policy = policy};
    int black = 0, h = visit(&x, c->root, -1, INT64_MIN, INT64_MAX, &black);
    unsigned count = 0;
    for (unsigned i = 0; i < c->capacity; ++i)
    {
        if (x.seen[i] != (c->nodes[i].live != 0))
        {
            x.failed = 1;
        }
        count += x.seen[i];
    }
    if (count != c->count ||
        (policy == 2 && c->root >= 0 && c->root < (int)c->capacity &&
         c->nodes[c->root].red))
    {
        x.failed = 1;
    }
    if (x.failed)
    {
        return ALGO_BAD;
    }
    *out = h;
    return ALGO_OK;
}
