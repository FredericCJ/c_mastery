#include "cancellation.h"
AeCode ae_operation_start(AeOperation *o, const unsigned char *p, unsigned n,
                          AeTerminal t, void *c, unsigned *g) {
  (void)o;
  (void)p;
  (void)n;
  (void)t;
  (void)c;
  (void)g;
  return AE_UNIMPLEMENTED;
}
AeCode ae_operation_event(AeOperation *o, unsigned g, AeEvent e) {
  (void)o;
  (void)g;
  (void)e;
  return AE_UNIMPLEMENTED;
}
