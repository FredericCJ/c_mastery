#include "scheduler.h"

int ss_portable_submit(struct ps_spi *s, struct ps_io io, uint32_t generation,
                       uint32_t token, const uint8_t *tx, uint8_t *rx,
                       const uint16_t *tw, uint16_t *rw, unsigned n,
                       unsigned dma, uint32_t now, unsigned budget,
                       unsigned setup, unsigned hold)
{
    return ps_begin(s, io, generation, token, dma ? (const uint8_t *)tw : tx,
                    dma ? (uint8_t *)rw : rx, n, dma, now, budget, setup, hold);
}

int ss_f439_submit(struct ps_spi *s, struct ps_io io, uint32_t generation,
                   uint32_t token, const uint8_t *tx, uint8_t *rx,
                   const uint16_t *tw, uint16_t *rw, unsigned n, unsigned dma,
                   uint32_t now, unsigned budget, unsigned setup, unsigned hold)
{
    (void)io;
    return dma ? ps_f439_begin_dma(s, generation, token, tw, rw, n, now, budget,
                                   setup, hold)
               : ps_f439_begin(s, generation, token, tx, rx, n, 0, now, budget,
                               setup, hold);
}
