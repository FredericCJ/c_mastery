#include "i2c.h"
/* Intentionally unfinished learner scaffold. */
int pi_open(struct pi_i2c *s, struct pi_io io, const struct pu_clock *c) {
  (void)s;
  (void)io;
  (void)c;
  return PI_UNIMPLEMENTED;
}
int pi_begin(struct pi_i2c *s, struct pi_io io, uint32_t generation, uint32_t token,
             unsigned address, unsigned command, unsigned count, uint8_t *out,
             uint32_t now, unsigned budget) {
  (void)s;
  (void)io;
  (void)generation;
  (void)token;
  (void)address;
  (void)command;
  (void)count;
  (void)out;
  (void)now;
  (void)budget;
  return PI_UNIMPLEMENTED;
}
int pi_cancel(struct pi_i2c *s, struct pi_io io, uint32_t token) {
  (void)s;
  (void)io;
  (void)token;
  return PI_UNIMPLEMENTED;
}
int pi_step(struct pi_i2c *s, struct pi_io io, uint32_t token, uint32_t now) {
  (void)s;
  (void)io;
  (void)token;
  (void)now;
  return PI_UNIMPLEMENTED;
}
int pi_recover_begin(struct pi_recovery *s, struct pi_lines io, unsigned exclusive,
                     unsigned peers, uint32_t now, unsigned budget,
                     unsigned half) {
  (void)s;
  (void)io;
  (void)exclusive;
  (void)peers;
  (void)now;
  (void)budget;
  (void)half;
  return PI_UNIMPLEMENTED;
}
int pi_recover_step(struct pi_recovery *s, struct pi_lines io, uint32_t now) {
  (void)s;
  (void)io;
  (void)now;
  return PI_UNIMPLEMENTED;
}
int pi_device(unsigned command, unsigned lose, uint8_t *device,
              uint8_t *reply) {
  (void)command;
  (void)lose;
  (void)device;
  (void)reply;
  return PI_UNIMPLEMENTED;
}
int pi_retry_policy(unsigned command, unsigned applied, unsigned status) {
  (void)command;
  (void)applied;
  (void)status;
  return PI_UNIMPLEMENTED;
}
