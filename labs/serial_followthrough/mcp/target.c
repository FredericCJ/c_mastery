#include "spi_adapter.h"

int mc_f439_open(struct mc_spi *a, const struct pu_clock *b, void *c,
                 uint32_t (*d)(void *))
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return PS_UNIMPLEMENTED;
}
