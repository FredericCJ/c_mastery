#include "uart.h"
/* Intentionally incomplete learner implementation; public tests must reject it.
 */
int pu_rates(const struct pu_clock *c, struct pu_rates *r) {
  (void)c;
  (void)r;
  return PU_UNIMPLEMENTED;
}
int pu_peer_ready(const struct pu_rx *q, unsigned *ready) {
  (void)q;
  (void)ready;
  return PU_UNIMPLEMENTED;
}
int pu_receive(struct pu_rx *q, unsigned status, uint8_t value) {
  (void)q;
  (void)status;
  (void)value;
  return PU_UNIMPLEMENTED;
}
int pu_take(struct pu_rx *q, struct pu_byte *v) {
  (void)q;
  (void)v;
  return PU_UNIMPLEMENTED;
}
int pu_open(struct pu_driver *d, struct pu_io io, const struct pu_clock *c, uint32_t baud,
            unsigned over8, unsigned polls) {
  (void)d;
  (void)io;
  (void)c;
  (void)baud;
  (void)over8;
  (void)polls;
  return PU_UNIMPLEMENTED;
}
int pu_submit(struct pu_driver *d, struct pu_io io, uint32_t gen, uint32_t token,
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
int pu_cancel(struct pu_driver *d, struct pu_io io, uint32_t token) {
  (void)d;
  (void)io;
  (void)token;
  return PU_UNIMPLEMENTED;
}
int pu_reset_notice(struct pu_driver *d, struct pu_io io) {
  (void)d;
  (void)io;
  return PU_UNIMPLEMENTED;
}
int pu_step(struct pu_driver *d, struct pu_io io, uint32_t token, uint32_t now) {
  (void)d;
  (void)io;
  (void)token;
  (void)now;
  return PU_UNIMPLEMENTED;
}
int pu_poll(struct pu_driver *d, struct pu_io io, uint32_t token, uint32_t now,
            unsigned polls) {
  (void)d;
  (void)io;
  (void)token;
  (void)now;
  (void)polls;
  return PU_UNIMPLEMENTED;
}
int pu_rx_poll(struct pu_rx *q, struct pu_io io, unsigned polls) {
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
