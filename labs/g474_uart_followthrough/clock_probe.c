#include "port.h"

/* Original supplied measurement adapter. No HAL, LL, libc or hidden timebase.
   Selected MCO is HSI16/16 on PA8 AF0. This is a second, explicitly owned pin. */
static void field(struct g4_io io, enum g4_reg r, uint32_t mask, uint32_t value)
{
    io.write(io.context, r, (io.read(io.context, r) & ~mask) | value);
}

int g4_clock_probe(struct g4_io io)
{
    if (!io.read || !io.write)
        return G4_INVALID;
    if (!(io.read(io.context, G4_RCC_CR) & (1u << 10)))
        return G4_CLOCK_TIMEOUT;
    field(io, G4_GPIO_OTYPER, 1u << 8, 0);
    field(io, G4_GPIO_OSPEEDR, 3u << 16, 2u << 16);
    field(io, G4_GPIO_PUPDR, 3u << 16, 0);
    field(io, G4_GPIO_AFRH, 15u, 0);
    field(io, G4_GPIO_MODER, 3u << 16, 2u << 16);
    field(io, G4_RCC_CFGR, 0x7f000000u, 0x43000000u);
    return G4_OK;
}
