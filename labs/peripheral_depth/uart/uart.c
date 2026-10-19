#include "uart.h"
/* Intentionally incomplete learner implementation; public tests must reject it.
 */
int pu_rates(const PuClock *c, PuRates *r) {
  (void)c;
  (void)r;
  return PU_UNIMPLEMENTED;
}
int pu_peer_ready(const PuRx *q, unsigned *ready) {
  (void)q;
  (void)ready;
  return PU_UNIMPLEMENTED;
}
int pu_receive(PuRx *q, unsigned status, uint8_t value) {
  (void)q;
  (void)status;
  (void)value;
  return PU_UNIMPLEMENTED;
}
int pu_take(PuRx *q, PuByte *v) {
  (void)q;
  (void)v;
  return PU_UNIMPLEMENTED;
}
int pu_open(PuDriver *d, PuIo io, const PuClock *c, uint32_t baud,
            unsigned over8, unsigned polls) {
  (void)d;
  (void)io;
  (void)c;
  (void)baud;
  (void)over8;
  (void)polls;
  return PU_UNIMPLEMENTED;
}
int pu_submit(PuDriver *d, PuIo io, uint32_t gen, uint32_t token,
              unsigned backend, const uint8_t *buf, unsigned n, uint32_t now,
              uint32_t budget) {
  (void)d;
  (void)io;
  (void)gen;
  (void)token;
  (void)backend;
  (void)buf;
  (void)n;
  (void)now;
  (void)budget;
  return PU_UNIMPLEMENTED;
}
int pu_cancel(PuDriver *d, PuIo io, uint32_t token) {
  (void)d;
  (void)io;
  (void)token;
  return PU_UNIMPLEMENTED;
}
int pu_reset_notice(PuDriver *d, PuIo io) {
  (void)d;
  (void)io;
  return PU_UNIMPLEMENTED;
}
int pu_step(PuDriver *d, PuIo io, uint32_t token, uint32_t now) {
  (void)d;
  (void)io;
  (void)token;
  (void)now;
  return PU_UNIMPLEMENTED;
}
int pu_poll(PuDriver *d, PuIo io, uint32_t token, uint32_t now,
            unsigned polls) {
  (void)d;
  (void)io;
  (void)token;
  (void)now;
  (void)polls;
  return PU_UNIMPLEMENTED;
}
int pu_rx_poll(PuRx *q, PuIo io, unsigned polls) {
  (void)q;
  (void)io;
  (void)polls;
  return PU_UNIMPLEMENTED;
}
int pu_deadline_fit(uint32_t clock, uint32_t divisor, unsigned n, uint32_t us,
                    unsigned *fits) {
  (void)clock;
  (void)divisor;
  (void)n;
  (void)us;
  (void)fits;
  return PU_UNIMPLEMENTED;
}
