#include "cmx_advanced.h"
void cmx_identity_init(cmx_identity_pool *p) { (void)p; }
uint32_t cmx_identity_take(cmx_identity_pool *p) {
  (void)p;
  return 0;
}
int cmx_identity_valid(const cmx_identity_pool *p, uint32_t h) {
  (void)p;
  (void)h;
  return 0;
}
enum cmx_result cmx_identity_put(cmx_identity_pool *p, uint32_t h) {
  (void)p;
  (void)h;
  return CMX_INVALID;
}
