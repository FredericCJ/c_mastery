#include "object_runtime_marking.h"

void orcm_init(struct orc_heap *h, enum orc_policy p)
{
    (void)h;
    (void)p;
}

int orcm_invariant(const struct orc_heap *h)
{
    (void)h;
    return 0;
}

enum or_status orcm_allocate(struct orc_heap *h, struct orc_handle a,
                             struct orc_handle b, struct orc_handle *o)
{
    (void)h;
    (void)a;
    (void)b;
    (void)o;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_retain(struct orc_heap *h, struct orc_handle p)
{
    (void)h;
    (void)p;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_release(struct orc_heap *h, struct orc_handle p)
{
    (void)h;
    (void)p;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_store(struct orc_heap *h, struct orc_handle o, unsigned f,
                          struct orc_handle t)
{
    (void)h;
    (void)o;
    (void)f;
    (void)t;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_read(const struct orc_heap *h, struct orc_handle o,
                         struct orc_handle e[2])
{
    (void)h;
    (void)o;
    (void)e;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_begin(struct orc_heap *h)
{
    (void)h;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_step(struct orc_heap *h, unsigned n, unsigned *s)
{
    (void)h;
    (void)n;
    (void)s;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_prepare(const struct orc_heap *h, struct orc_finish *t)
{
    (void)h;
    (void)t;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_commit(struct orc_heap *h, struct orc_finish t, uint16_t *d)
{
    (void)h;
    (void)t;
    (void)d;
    return OR_UNIMPLEMENTED;
}
