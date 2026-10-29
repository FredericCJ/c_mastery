#include "clock.h"

int xk_snapshot(struct xk_io a, struct xk_policy b, struct xk_snapshot *c)
{
    (void)a;
    (void)b;
    (void)c;
    return XK_UNIMPLEMENTED;
}

int xk_sequence(struct xk_sequence *a, uint32_t b, uint32_t c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return XK_UNIMPLEMENTED;
}

int xk_calibrate(const struct xk_calibration *a, uint64_t b, uint64_t c,
                 struct xk_interval *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return XK_UNIMPLEMENTED;
}

int xk_event(struct xk_interval a, uint64_t b, uint64_t c,
             struct xk_interval *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return XK_UNIMPLEMENTED;
}

int xk_order(struct xk_interval a, struct xk_interval b, unsigned *c)
{
    (void)a;
    (void)b;
    (void)c;
    return XK_UNIMPLEMENTED;
}
