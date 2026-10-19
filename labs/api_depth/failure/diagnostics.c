#include "diagnostics.h"
unsigned ae_threshold = 2;
void ae_developer_emit(unsigned x) { (void)x; }
void ae_integrator_emit(unsigned s, unsigned x) {
  (void)s;
  (void)x;
}
AeCode ae_production_capacity(unsigned n) {
  (void)n;
  return AE_UNIMPLEMENTED;
}
unsigned ae_diagnostic_events(unsigned c) {
  (void)c;
  return 0;
}
