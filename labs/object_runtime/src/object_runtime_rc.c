#include "object_runtime_rc.h"

void orr_init(struct orr_heap *h)
{
    if (h)
        *h = (struct orr_heap){0};
}

int orr_invariant(const struct orr_heap *h)
{
    (void)h;
    return 0;
}

enum or_status orr_new(struct orr_heap *h, struct orr_handle *out)
{
    (void)h;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_retain(struct orr_heap *h, struct orr_handle a)
{
    (void)h;
    (void)a;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_release(struct orr_heap *h, struct orr_handle a, int defer,
                           uint16_t *out)
{
    (void)h;
    (void)a;
    (void)defer;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_store(struct orr_heap *h, struct orr_handle a,
                         unsigned field, struct orr_handle b, int defer,
                         uint16_t *out)
{
    (void)h;
    (void)a;
    (void)field;
    (void)b;
    (void)defer;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_flush(struct orr_heap *h, unsigned budget,
                         unsigned *processed, uint16_t *out)
{
    (void)h;
    (void)budget;
    (void)processed;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_prepare(struct orr_heap *h, struct orr_trial *out)
{
    (void)h;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_commit(struct orr_heap *h, const struct orr_trial *t,
                          uint16_t *out)
{
    (void)h;
    (void)t;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orr_close(struct orr_heap *h)
{
    (void)h;
    return OR_UNIMPLEMENTED;
}
