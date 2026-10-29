#include "object_runtime_finalizers.h"

void orf_init(struct orf_heap *h)
{
    if (h)
    {
        *h = (struct orf_heap){0};
    }
}

enum or_status orf_new(struct orf_heap *h, orf_finalizer_fn cb, void *ctx,
                       int resource, int fail, struct orf_handle *out)
{
    (void)h;
    (void)cb;
    (void)ctx;
    (void)resource;
    (void)fail;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_root(struct orf_heap *h, struct orf_handle a, int present)
{
    (void)h;
    (void)a;
    (void)present;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_edge(struct orf_heap *h, struct orf_handle a, unsigned field,
                        struct orf_handle b)
{
    (void)h;
    (void)a;
    (void)field;
    (void)b;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_pair(struct orf_heap *h, unsigned index, struct orf_handle a,
                        struct orf_handle b)
{
    (void)h;
    (void)index;
    (void)a;
    (void)b;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_weak(struct orf_heap *h, unsigned index, struct orf_handle a)
{
    (void)h;
    (void)index;
    (void)a;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_collect(struct orf_heap *h, uint16_t *out)
{
    (void)h;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_run_one(struct orf_heap *h, struct orf_handle *out,
                           enum or_status *status)
{
    (void)h;
    (void)out;
    (void)status;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_close_resource(struct orf_heap *h, struct orf_handle a)
{
    (void)h;
    (void)a;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_cleanup(struct orf_heap *h, unsigned budget, unsigned *out)
{
    (void)h;
    (void)budget;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orf_close(struct orf_heap *h)
{
    (void)h;
    return OR_UNIMPLEMENTED;
}
