#ifndef MC_SPI_ADAPTER_H
#define MC_SPI_ADAPTER_H
#include "mcp.h"
#include "spi.h"
typedef struct {
  PsSpi spi;
  PsIo io;
  void *clock_context;
  uint32_t (*ticks)(void *);
  uint32_t token;
  unsigned budget, setup, hold;
  uint8_t tx[16], rx[16];
} McSpi;
/* Open the retained PsSpi first in mode0, PIO, <=1MHz. Driver owns staging;
   exhausted/stuck adapter cannot be reopened before physical quiescence. */
int mc_spi_transfer(void *, const uint8_t *, uint8_t *, size_t);
uint32_t mc_spi_now(void *);
int mc_f439_open(McSpi *, const PuClock *, void *, uint32_t (*)(void *));
#endif
