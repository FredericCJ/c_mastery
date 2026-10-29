#include "owned_stream.h"

int cf_item_drop(struct cf_item *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int cf_map_next(struct cf_source *a, cf_map_fn b, void *c, struct cf_item *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return CO_UNIMPLEMENTED;
}

int cf_zip_next(struct cf_source *a, struct cf_source *b, cf_map_fn c, void *d,
                struct cf_item *e, struct cf_item *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return CO_UNIMPLEMENTED;
}

int cf_gen_create(struct co_allocator a, unsigned b, struct cf_generator **c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int cf_gen_next(struct cf_generator *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int cf_gen_borrow(struct cf_generator *a, const unsigned char **b, size_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int cf_gen_take(struct cf_generator *a, struct cf_item *b)
{
    (void)a;
    (void)b;
    return CO_UNIMPLEMENTED;
}

int cf_gen_clone(struct cf_generator *a, struct cf_item *b)
{
    (void)a;
    (void)b;
    return CO_UNIMPLEMENTED;
}

int cf_gen_destroy(struct cf_generator **a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}
