#include "support.h"
int lc_host_backend(LCBackend *p, int value) {
  ++p->calls;
  return value + p->bias;
}
int lc_backend_run(LCBackendOps ops, int value) {
  return ops.apply(ops.context, value);
}
