#ifndef PERIPHERAL_SPI_DEPTH_H
#define PERIPHERAL_SPI_DEPTH_H
#include "uart.h"
#include <stddef.h>
#include <stdint.h>
enum {
  PS_OK,
  PS_INVALID,
  PS_BUSY,
  PS_STALE,
  PS_TIMEOUT,
  PS_IO,
  PS_CANCELLED,
  PS_PROTOCOL,
  PS_UNIMPLEMENTED
};
enum {
  PS_SR,
  PS_DR,
  PS_CS,
  PS_CONFIG,
  PS_ENABLE,
  PS_TX_EN,
  PS_RX_EN,
  PS_TX_COUNT,
  PS_RX_COUNT,
  PS_TX_FLAGS,
  PS_RX_FLAGS,
  PS_TX_ADDR,
  PS_RX_ADDR,
  PS_DMA_CLEAR,
  PS_RX_START,
  PS_TX_START,
  PS_DMA_REQUESTS,
  PS_RESET
};
enum {
  PS_RXNE = 1,
  PS_TXE = 2,
  PS_CRCERR = 16,
  PS_MODF = 32,
  PS_OVR = 64,
  PS_BSY = 128
};
struct ps_io {
  void *context;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uintptr_t);
};
struct ps_spi {
  uint32_t generation, clock, last_token, token, last, deadline, due;
  unsigned ready, mode, divider, phase, dma, count, sent, received, tx_owned,
      rx_owned, cs_owned, hold, result, dma_started;
  const uint8_t *tx;
  uint8_t *rx;
};
int ps_open(struct ps_spi *, struct ps_io, const struct pu_clock *, unsigned mode, unsigned divider);
int ps_begin(struct ps_spi *, struct ps_io, uint32_t generation, uint32_t token,
             const uint8_t *tx, uint8_t *rx, unsigned count, unsigned dma,
             uint32_t now, unsigned budget, unsigned setup, unsigned hold);
int ps_step(struct ps_spi *, struct ps_io, uint32_t token, uint32_t now);
int ps_cancel(struct ps_spi *, struct ps_io, uint32_t token);
int ps_abort_reset(struct ps_spi *, struct ps_io, uint32_t token, uint32_t now);
/* Original teaching sensor wire protocol; no real product compatibility. */
int ps_command(unsigned version, unsigned reg, uint8_t out[3]);
int ps_decode(unsigned version, const uint8_t in[3], uint16_t *value);
struct ps_io ps_f439_io(void);
void ps_f439_pins(void);
int ps_f439_begin(struct ps_spi *, uint32_t generation, uint32_t token, const uint8_t *,
                  uint8_t *, unsigned count, unsigned dma, uint32_t now,
                  unsigned budget, unsigned setup, unsigned hold);
/* DMA uses one aligned halfword per wire octet, low8 bits significant. */
int ps_f439_begin_dma(struct ps_spi *, uint32_t generation, uint32_t token,
                      const uint16_t *, uint16_t *, unsigned count,
                      uint32_t now, unsigned budget, unsigned setup,
                      unsigned hold);
int ps_pack(const uint8_t *, uint16_t *, unsigned count);
int ps_unpack(const uint16_t *, uint8_t *, unsigned count);
#endif
