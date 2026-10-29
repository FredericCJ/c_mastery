#include "doorbell.h"

int af_ring_door(struct af_registers a, struct af_door b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_mask_door(struct af_registers a, struct af_door b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return AF_UNIMPLEMENTED;
}

int af_clear_door(struct af_registers a, struct af_door b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_service_door(struct af_registers a, struct af_door b, struct af_ring *c,
                    uint32_t d, unsigned e, unsigned *f, unsigned *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return AF_UNIMPLEMENTED;
}
