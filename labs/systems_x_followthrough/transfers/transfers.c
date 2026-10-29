#include "transfers.h"

/* Intentionally unimplemented: explicit completed baseline providers required. */
int sx_remap(struct sx_map *a, unsigned b, unsigned c, unsigned d, unsigned e,
             unsigned char f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return B5_UNIMPLEMENTED;
}

int sx_copy(struct sx_map *a, size_t b, const unsigned char *c, size_t d,
            sx_between_fn e, void *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return B5_UNIMPLEMENTED;
}

int sx_partial(struct sx_map *a, size_t b, const unsigned char *c, size_t d,
               struct sx_copy_result *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return B5_UNIMPLEMENTED;
}

int sx_allocate(struct sx_ledger *a, uint32_t b, size_t c, unsigned d,
                unsigned e, struct sx_alias *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return B4_UNIMPLEMENTED;
}

int sx_resize(struct sx_ledger *a, struct sx_alias b, unsigned c, size_t d,
              int e, struct sx_alias *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return B4_UNIMPLEMENTED;
}

int sx_transfer(struct sx_ledger *a, struct sx_alias b, unsigned c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B4_UNIMPLEMENTED;
}

int sx_release(struct sx_ledger *a, struct sx_alias b, unsigned c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B4_UNIMPLEMENTED;
}

int sx_lookup(const struct sx_ledger *a, struct sx_alias b, unsigned c,
              size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return B4_UNIMPLEMENTED;
}

int sx_online(struct sx_hotplug *a, unsigned b)
{
    (void)a;
    (void)b;
    return S_UNIMPLEMENTED;
}

int sx_begin(struct sx_hotplug *a, unsigned b)
{
    (void)a;
    (void)b;
    return S_UNIMPLEMENTED;
}

int sx_offline_request(struct sx_hotplug *a, unsigned b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return S_UNIMPLEMENTED;
}

int sx_offline_quiescent(struct sx_hotplug *a, unsigned b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return S_UNIMPLEMENTED;
}

int sx_ack(struct sx_hotplug *a, unsigned b, uint32_t c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return S_UNIMPLEMENTED;
}

int sx_reclaim(struct sx_hotplug *a)
{
    (void)a;
    return S_UNIMPLEMENTED;
}
