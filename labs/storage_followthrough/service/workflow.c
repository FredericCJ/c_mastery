#include "workflow.h"

int sw_create(struct sw_flow *f, store *s, struct vg_vh *a, struct vg_vh *b,
              int32_t n, unsigned d)
{
    (void)f;
    (void)s;
    (void)a;
    (void)b;
    (void)n;
    (void)d;
    return SW_UNIMPLEMENTED;
}

int sw_open(struct sw_flow *f, store *s, struct vg_vh *a, struct vg_vh *b)
{
    (void)f;
    (void)s;
    (void)a;
    (void)b;
    return SW_UNIMPLEMENTED;
}

int sw_step(struct sw_flow *f)
{
    (void)f;
    return SW_UNIMPLEMENTED;
}
