#include "dma.h"

int pd_validate(unsigned a, unsigned b, const struct pd_extent *c, unsigned d,
                uint32_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return PD_UNIMPLEMENTED;
}

int pd_submit(struct pd_transfer *a, struct pd_io b, unsigned c, unsigned d,
              const struct pd_extent *e, unsigned f, uint32_t g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return PD_UNIMPLEMENTED;
}

int pd_poll(struct pd_transfer *a, struct pd_io b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return PD_UNIMPLEMENTED;
}

int pd_cancel(struct pd_transfer *a, struct pd_io b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return PD_UNIMPLEMENTED;
}

int pd_finalize(struct pd_transfer *a, struct pd_io b, uint32_t c,
                struct pd_result *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return PD_UNIMPLEMENTED;
}

int pd_suffix(const struct pd_extent *a, unsigned b, uint32_t c,
              struct pd_extent *d, unsigned *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return PD_UNIMPLEMENTED;
}
