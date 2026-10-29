#include "dod.h"

/* Deliberately incomplete learner mechanism bodies. Initializers establish safe empty state. */
void d_small_init(struct d_small *s, struct d_allocator a)
{
    s->heap = 0;
    s->length = 0;
    s->capacity = 16;
    s->allocator = a;
}

const unsigned char *d_small_data(const struct d_small *s)
{
    return s->heap ? s->heap : s->local;
}

void d_sparse_init(struct d_sparse *s)
{
    unsigned i;
    s->count = 0;
    for (i = 0; i < 4; i++)
    {
        s->slots[i] = (struct d_slot){1, 0, 0};
        s->rank[i] = 0;
        s->dense[i] = 0;
    }
}

void d_ecs_init(struct d_ecs *e)
{
    unsigned i;
    for (i = 0; i < 3; i++)
    {
        e->tables[i].count = 0;
    }
    for (i = 0; i < 4; i++)
    {
        e->mask[i] = 0;
        e->row[i] = 0;
    }
}

void d_tree_init(struct d_tree *t)
{
    t->used = 0;
    t->fail_after = 0;
    t->allocations = 0;
}

void d_hamt_init(struct d_hamt *h)
{
    h->used = 0;
    h->fail_after = 0;
    h->allocations = 0;
}

void d_rope_init(struct d_rope *r)
{
    r->used = 0;
}

int d_small_append(struct d_small *arg0, const unsigned char *arg1, size_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return D_INVALID;
}

int d_small_truncate(struct d_small *arg0, size_t arg1)
{
    (void)arg0;
    (void)arg1;
    return D_INVALID;
}

int d_small_compact(struct d_small *arg0)
{
    (void)arg0;
    return D_INVALID;
}

int d_small_move(struct d_small *arg0, struct d_small *arg1)
{
    (void)arg0;
    (void)arg1;
    return D_INVALID;
}

void d_small_reset(struct d_small *arg0)
{
    (void)arg0;
}

int d_columns_step(struct d_columns *arg0, int32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return D_INVALID;
}

uint32_t d_swar_add(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return 0;
}

int d_sparse_add(struct d_sparse *arg0, int arg1, struct d_handle *arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return D_INVALID;
}

int d_sparse_get(const struct d_sparse *arg0, struct d_handle arg1, int *arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return D_INVALID;
}

int d_sparse_remove(struct d_sparse *arg0, struct d_handle arg1)
{
    (void)arg0;
    (void)arg1;
    return D_INVALID;
}

int d_ecs_set(struct d_ecs *arg0, unsigned arg1, unsigned arg2, int arg3,
              int arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    return D_INVALID;
}

int d_ecs_step(struct d_ecs *arg0)
{
    (void)arg0;
    return D_INVALID;
}

int d_tree_update(struct d_tree *arg0, unsigned arg1, unsigned arg2, int arg3,
                  unsigned *arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    return D_INVALID;
}

int d_tree_get(const struct d_tree *arg0, unsigned arg1, unsigned arg2,
               int *arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return D_INVALID;
}

int d_hamt_update(struct d_hamt *arg0, unsigned arg1, unsigned arg2, int arg3,
                  unsigned *arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    return D_INVALID;
}

int d_hamt_get(const struct d_hamt *arg0, unsigned arg1, unsigned arg2,
               int *arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return D_INVALID;
}

int d_rope_leaf(struct d_rope *arg0, const unsigned char *arg1, size_t arg2,
                unsigned *arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return D_INVALID;
}

int d_rope_concat(struct d_rope *arg0, unsigned arg1, unsigned arg2,
                  unsigned *arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return D_INVALID;
}

int d_rope_flatten(const struct d_rope *arg0, unsigned arg1,
                   unsigned char *arg2, size_t arg3, size_t *arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    return D_INVALID;
}
