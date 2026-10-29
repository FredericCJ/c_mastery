#include "tree.h"

void at_init(struct at_pool *p)
{
    (void)p;
}

int at_insert(struct at_pool *p, int32_t k, unsigned f, struct at_work *w)
{
    (void)p;
    (void)k;
    (void)f;
    (void)w;
    return ALGO_UNIMPLEMENTED;
}

int at_delete(struct at_pool *p, int32_t k, unsigned f, struct at_work *w)
{
    (void)p;
    (void)k;
    (void)f;
    (void)w;
    return ALGO_UNIMPLEMENTED;
}

int at_contains(struct at_pool *p, int32_t k, unsigned f, struct at_work *w)
{
    (void)p;
    (void)k;
    (void)f;
    (void)w;
    return 0;
}

int at_certificate(const struct at_pool *p, struct at_certificate *c)
{
    (void)p;
    (void)c;
    return ALGO_UNIMPLEMENTED;
}

int at_cd_certificate(const struct cd_tree *p, struct at_certificate *c)
{
    (void)p;
    (void)c;
    return ALGO_UNIMPLEMENTED;
}

int at_check(const struct at_certificate *c, unsigned p, int *h)
{
    (void)c;
    (void)p;
    (void)h;
    return ALGO_UNIMPLEMENTED;
}

/* Required prior artifacts are separate unsolved import placeholders. */
int algo_avl_insert(algo_avl *t, int32_t k)
{
    (void)t;
    (void)k;
    return ALGO_UNIMPLEMENTED;
}

int algo_avl_contains(const algo_avl *t, int32_t k)
{
    (void)t;
    (void)k;
    return 0;
}

void cd_tree_init(struct cd_tree *t, unsigned b, unsigned m)
{
    (void)t;
    (void)b;
    (void)m;
}

enum cd_status cd_tree_insert(struct cd_tree *t, int k, unsigned f, int *s)
{
    (void)t;
    (void)k;
    (void)f;
    (void)s;
    return CD_FAILED;
}

enum cd_status cd_tree_delete(struct cd_tree *t, int k)
{
    (void)t;
    (void)k;
    return CD_FAILED;
}
