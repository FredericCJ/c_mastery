#include "spi_adapter.h"
#include <string.h>
#ifndef MC_MUTANT
#define MC_MUTANT 0
#endif
uint32_t mc_spi_now(void *v)
{
    struct mc_spi *a = v;
    return a->ticks(a->clock_context);
}

int mc_spi_transfer(void *v, const uint8_t *tx, uint8_t *rx, size_t n)
{
    struct mc_spi *a = v;
    if (!a || !a->ticks || !tx || !rx || !n || n > 16 || a->spi.phase ||
        a->spi.cs_owned || a->spi.mode != 0 || a->token == UINT32_MAX)
        return -1;
    memcpy(a->tx, tx, n);
    uint32_t token = ++a->token;
    int r =
        ps_begin(&a->spi, a->io, a->spi.generation, token, a->tx, a->rx,
                 (unsigned)n, 0, mc_spi_now(a), a->budget, a->setup, a->hold);
    if (r != PS_OK)
        return -1;
    for (unsigned i = 0; i < 4096 && a->spi.phase; i++)
    {
        uint32_t now = mc_spi_now(a);
        r = ps_step(&a->spi, a->io, token, now);
        if (a->spi.phase == 4)
            (void)ps_abort_reset(&a->spi, a->io, token, now);
    }
    if (a->spi.phase)
    {
        (void)ps_cancel(&a->spi, a->io, token);
        return MC_MUTANT == 9 ? 0 : -1;
    }
    if (r != PS_OK)
        return -1;
    memcpy(rx, a->rx, n);
    return 0;
}
