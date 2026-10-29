#include "uart_adapter.h"
#ifndef PL_ADAPTER_MUTANT
#define PL_ADAPTER_MUTANT 0
#endif
static int result(int r) {
  switch (r) {
  case PU_OK:
    return PL_OK;
  case PU_BUSY:
    return PL_BUSY;
  case PU_STALE:
    return PL_STALE;
  case PU_TIMEOUT:
    return PL_TIMEOUT;
  case PU_CANCELLED:
    return PL_CANCELLED;
  case PU_INVALID:
    return PL_INVALID;
  case PU_IO:
    return PL_IO;
  default:
    /* Unknown/full/unimplemented is not a certified terminal I/O failure. */
    return PL_ADAPTER_MUTANT == 1 ? PL_IO : PL_INVALID;
  }
}
int pl_uart_open(struct pl_uart *u, struct pu_io io, const struct pu_clock *clock, uint32_t baud,
                 unsigned backend, unsigned polls) {
  if (!u || !clock || backend > PU_DMA)
    return PL_INVALID;
  int r = pu_open(&u->driver, io, clock, baud, 0, polls);
  if (r == PU_OK) {
    u->io = io;
    u->generation = clock->generation;
    u->backend = backend;
  }
  return result(r);
}
static int submit(void *ctx, const uint8_t *bytes, unsigned n, uint32_t token,
                  uint32_t now, uint32_t budget) {
  struct pl_uart *u = ctx;
  return result(pu_submit(&u->driver, u->io, u->generation, token, u->backend,
                          bytes, n, now, budget));
}
static int poll(void *ctx, uint32_t token, uint32_t now) {
  struct pl_uart *u = ctx;
  return result(pu_step(&u->driver, u->io, token, now));
}
static int cancel(void *ctx, uint32_t token) {
  struct pl_uart *u = ctx;
  return result(pu_cancel(&u->driver, u->io, token));
}
struct pl_transport pl_uart_transport(struct pl_uart *u) {
  if (!u) {
    struct pl_transport empty = {0};
    return empty;
  }
  struct pl_transport t = {u, submit, poll, cancel};
  return t;
}
