#include "compare.h"
int pl_byte(PlBytePort p, uint8_t b, uint32_t t) {
  (void)p;
  (void)b;
  (void)t;
  return PL_UNIMPLEMENTED;
}
PlBytePort pl_direct_f439(volatile uint32_t *p, PlTick t, void *c) {
  (void)p;
  (void)t;
  (void)c;
  return (PlBytePort){0};
}
PlBytePort pl_direct_g474(volatile uint32_t *p, PlTick t, void *c) {
  (void)p;
  (void)t;
  (void)c;
  return (PlBytePort){0};
}
PlBytePort pl_generated(volatile uint32_t *p, unsigned r, PlTick t, void *c) {
  (void)p;
  (void)r;
  (void)t;
  (void)c;
  return (PlBytePort){0};
}
