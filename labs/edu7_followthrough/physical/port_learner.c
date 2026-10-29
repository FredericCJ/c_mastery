#include "physical.h"

int ep_f439_open(struct ep_tx *s, const struct pu_clock *c, uint32_t b,
                 unsigned d, uint32_t n)
{
    (void)s;
    (void)c;
    (void)b;
    (void)d;
    (void)n;
    return EP_STUB;
}

int ep_f439_open_port(struct ep_tx *s, const struct pu_clock *c, uint32_t b,
                      unsigned d, unsigned p, uint32_t n)
{
    (void)s;
    (void)c;
    (void)b;
    (void)d;
    (void)p;
    (void)n;
    return EP_STUB;
}
