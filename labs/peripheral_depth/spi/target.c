#include "spi.h"

/* Intentionally unfinished learner scaffold. */
struct ps_io ps_f439_io(void)
{
    struct ps_io io = {0, 0, 0};
    return io;
}

void ps_f439_pins(void)
{
}

int ps_f439_begin(struct ps_spi *s, uint32_t generation, uint32_t token,
                  const uint8_t *tx, uint8_t *rx, unsigned count, unsigned dma,
                  uint32_t now, unsigned budget, unsigned setup, unsigned hold)
{
    (void)s;
    (void)generation;
    (void)token;
    (void)tx;
    (void)rx;
    (void)count;
    (void)dma;
    (void)now;
    (void)budget;
    (void)setup;
    (void)hold;
    return PS_UNIMPLEMENTED;
}

int ps_f439_begin_dma(struct ps_spi *s, uint32_t generation, uint32_t token,
                      const uint16_t *tx, uint16_t *rx, unsigned count,
                      uint32_t now, unsigned budget, unsigned setup,
                      unsigned hold)
{
    (void)s;
    (void)generation;
    (void)token;
    (void)tx;
    (void)rx;
    (void)count;
    (void)now;
    (void)budget;
    (void)setup;
    (void)hold;
    return PS_UNIMPLEMENTED;
}
