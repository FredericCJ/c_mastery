#include "spi.h"
/* Intentionally unfinished learner scaffold. */
int ps_open(PsSpi *s, PsIo io, const PuClock *clock, unsigned mode,
            unsigned divider) {
  (void)s;
  (void)io;
  (void)clock;
  (void)mode;
  (void)divider;
  return PS_UNIMPLEMENTED;
}
int ps_begin(PsSpi *s, PsIo io, uint32_t generation, uint32_t token,
             const uint8_t *tx, uint8_t *rx, unsigned count, unsigned dma,
             uint32_t now, unsigned budget, unsigned setup, unsigned hold) {
  (void)s;
  (void)io;
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
int ps_cancel(PsSpi *s, PsIo io, uint32_t token) {
  (void)s;
  (void)io;
  (void)token;
  return PS_UNIMPLEMENTED;
}
int ps_abort_reset(PsSpi *s, PsIo io, uint32_t token, uint32_t now) {
  (void)s;
  (void)io;
  (void)token;
  (void)now;
  return PS_UNIMPLEMENTED;
}
int ps_step(PsSpi *s, PsIo io, uint32_t token, uint32_t now) {
  (void)s;
  (void)io;
  (void)token;
  (void)now;
  return PS_UNIMPLEMENTED;
}
int ps_command(unsigned version, unsigned reg, uint8_t out[3]) {
  (void)version;
  (void)reg;
  (void)out;
  return PS_UNIMPLEMENTED;
}
int ps_decode(unsigned version, const uint8_t in[3], uint16_t *out) {
  (void)version;
  (void)in;
  (void)out;
  return PS_UNIMPLEMENTED;
}
int ps_pack(const uint8_t *src, uint16_t *dst, unsigned count) {
  (void)src;
  (void)dst;
  (void)count;
  return PS_UNIMPLEMENTED;
}
int ps_unpack(const uint16_t *src, uint8_t *dst, unsigned count) {
  (void)src;
  (void)dst;
  (void)count;
  return PS_UNIMPLEMENTED;
}
