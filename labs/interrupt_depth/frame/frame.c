#include "frame.h"
int xf_frame(const uint32_t *w, size_t n, uint32_t sp, uint32_t er, unsigned f,
             unsigned m, unsigned a, unsigned fault, struct xf_frame *out) {
  (void)w;
  (void)n;
  (void)sp;
  (void)er;
  (void)f;
  (void)m;
  (void)a;
  (void)fault;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_priority(struct xf_priority_config c, unsigned r, struct xf_priority *out) {
  (void)c;
  (void)r;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_urgency(struct xf_priority_config c, unsigned u, unsigned o, uint8_t *out) {
  (void)c;
  (void)u;
  (void)o;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_api_allowed(struct xf_priority_config c, unsigned r, unsigned t, unsigned *out) {
  (void)c;
  (void)r;
  (void)t;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_pick(struct xf_priority_config c, const struct xf_irq *v, size_t n, int active,
            unsigned p, unsigned b, unsigned *out) {
  (void)c;
  (void)v;
  (void)n;
  (void)active;
  (void)p;
  (void)b;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_pend(struct xf_irq *v, unsigned b) {
  (void)v;
  (void)b;
  return XF_UNIMPLEMENTED;
}
int xf_source(struct xf_irq *v, unsigned b) {
  (void)v;
  (void)b;
  return XF_UNIMPLEMENTED;
}
