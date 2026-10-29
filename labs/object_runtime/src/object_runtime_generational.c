#include "object_runtime_generational.h"

enum or_status or_nursery_cards_init(const struct or_nursery *h,
                                     struct or_cards *out)
{
    (void)h;
    (void)out;
    return OR_STATE;
}

enum or_status or_nursery_old_store(struct or_nursery *h, struct or_cards *c,
                                    unsigned o, unsigned f, unsigned t)
{
    (void)h;
    (void)c;
    (void)o;
    (void)f;
    (void)t;
    return OR_STATE;
}

enum or_status or_minor_preflight(struct or_nursery *h, struct or_cards *c,
                                  unsigned y, unsigned o, unsigned *n,
                                  unsigned *s)
{
    (void)h;
    (void)c;
    (void)y;
    (void)o;
    (void)n;
    (void)s;
    return OR_STATE;
}

enum or_status or_major_preflight(struct or_nursery *h, struct or_cards *c,
                                  unsigned *n, uint8_t f[16])
{
    (void)h;
    (void)c;
    (void)n;
    (void)f;
    return OR_STATE;
}

void or_large_init(struct or_large_heap *h)
{
    (void)h;
}

enum or_status or_large_allocate(struct or_large_heap *h, unsigned n,
                                 uint32_t p, struct or_large_handle *o)
{
    (void)h;
    (void)n;
    (void)p;
    (void)o;
    return OR_STATE;
}

enum or_status or_large_release(struct or_large_heap *h,
                                struct or_large_handle p)
{
    (void)h;
    (void)p;
    return OR_STATE;
}

enum or_status or_large_pin(struct or_large_heap *h, struct or_large_handle p,
                            struct or_large_lease *o, const uint32_t **d,
                            unsigned *n)
{
    (void)h;
    (void)p;
    (void)o;
    (void)d;
    (void)n;
    return OR_STATE;
}

enum or_status or_large_unpin(struct or_large_lease p)
{
    (void)p;
    return OR_STATE;
}

enum or_status or_large_collect(struct or_large_heap *h, unsigned *n)
{
    (void)h;
    (void)n;
    return OR_STATE;
}
