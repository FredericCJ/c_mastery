#include "i2c.h"
/* Intentionally unfinished learner scaffold. */
PiIo pi_f439_io(void) {
  PiIo io = {0, 0, 0};
  return io;
}
void pi_f439_pins(void) {}
int pi_f439_step(PiI2c *s, uint32_t token, uint32_t now) {
  (void)s;
  (void)token;
  (void)now;
  return PI_UNIMPLEMENTED;
}
int pi_f439_open(PiI2c *s, const PuClock *clock) {
  (void)s;
  (void)clock;
  return PI_UNIMPLEMENTED;
}
int pi_f439_begin(PiI2c *s, uint32_t generation, uint32_t token,
                  unsigned address, unsigned command, unsigned count,
                  uint8_t *out, uint32_t now, unsigned budget) {
  (void)s;
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
int pi_f439_cancel(PiI2c *s, uint32_t token) {
  (void)s;
  (void)token;
  return PI_UNIMPLEMENTED;
}
PiLines pi_f439_lines(void) {
  PiLines io = {0, 0, 0};
  return io;
}
int pi_f439_recovery_prepare(PiI2c *s, unsigned exclusive, unsigned peers) {
  (void)s;
  (void)exclusive;
  (void)peers;
  return PI_UNIMPLEMENTED;
}
