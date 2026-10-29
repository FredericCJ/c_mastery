#include "spi_adapter.h"

/* Selected existing SPI1/PD14-CS route. Requires exclusive board ownership,
   actually valid clock generation and a calibrated monotonic microsecond tick.
   This code is cross-built, not executed on physical F439 by author campaigns.
 */
int mc_f439_open(struct mc_spi *a, const struct pu_clock *clock, void *ctx,
                 uint32_t (*ticks)(void *))
{
    struct pu_rates rates;
    if (!a || !clock || !ticks || pu_rates(clock, &rates) != PU_OK ||
        a->spi.phase || a->spi.cs_owned)
        return PS_INVALID;
    unsigned divider = 0;
    while (divider < 7 && rates.apb2 > (1000000u << (divider + 1)))
        divider++;
    if (rates.apb2 > (1000000u << (divider + 1)))
        return PS_INVALID;
    ps_f439_pins();
    a->io = ps_f439_io();
    a->clock_context = ctx;
    a->ticks = ticks;
    a->budget = 1000;
    a->setup = 2;
    a->hold = 2;
    return ps_open(&a->spi, a->io, clock, 0, divider);
}
