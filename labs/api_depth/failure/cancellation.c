#include "cancellation.h"
enum ae_code ae_operation_start(struct ae_operation *o, const unsigned char *p, unsigned n,
                          ae_terminal_fn t, void *c, unsigned *g) {
  (void)o;
  (void)p;
  (void)n;
  (void)t;
  (void)c;
  (void)g;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_operation_event(struct ae_operation *o, unsigned g, enum ae_event e) {
  (void)o;
  (void)g;
  (void)e;
  return AE_UNIMPLEMENTED;
}
