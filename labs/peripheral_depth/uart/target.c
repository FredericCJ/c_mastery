#include "uart.h"

/* Intentionally unfinished learner adapter: implement inspected F439 accesses.
 */
struct pu_io pu_f439_io(void)
{
    struct pu_io io = {0, 0, 0};
    return io;
}

void pu_f439_pins(void)
{
}

void pu_f439_rts_init(void)
{
}

int pu_f439_rts(const struct pu_rx *q)
{
    (void)q;
    return PU_UNIMPLEMENTED;
}

int pu_f439_dma_submit(struct pu_driver *d, uint32_t g, uint32_t t,
                       const uint8_t *p, unsigned n, uint32_t now,
                       uint32_t budget)
{
    (void)d;
    (void)g;
    (void)t;
    (void)p;
    (void)n;
    (void)now;
    (void)budget;
    return PU_UNIMPLEMENTED;
}
