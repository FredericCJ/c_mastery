#include "frame.h"
int xf_frame(const uint32_t *w, size_t n, uint32_t sp, uint32_t er, unsigned f,
             unsigned m, unsigned a, unsigned fault, XfFrame *out) {
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
int xf_priority(XfPriorityConfig c, unsigned r, XfPriority *out) {
  (void)c;
  (void)r;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_urgency(XfPriorityConfig c, unsigned u, unsigned o, uint8_t *out) {
  (void)c;
  (void)u;
  (void)o;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_api_allowed(XfPriorityConfig c, unsigned r, unsigned t, unsigned *out) {
  (void)c;
  (void)r;
  (void)t;
  (void)out;
  return XF_UNIMPLEMENTED;
}
int xf_pick(XfPriorityConfig c, const XfIrq *v, size_t n, int active,
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
int xf_pend(XfIrq *v, unsigned b) {
  (void)v;
  (void)b;
  return XF_UNIMPLEMENTED;
}
int xf_source(XfIrq *v, unsigned b) {
  (void)v;
  (void)b;
  return XF_UNIMPLEMENTED;
}
