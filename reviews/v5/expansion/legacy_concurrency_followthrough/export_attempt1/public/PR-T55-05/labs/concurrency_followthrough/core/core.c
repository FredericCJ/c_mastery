#include "core.h"
int cf_ring_init(CfRing *q, unsigned n, unsigned l, unsigned b) {
  (void)q;
  (void)n;
  (void)l;
  (void)b;
  return CF_UNIMPLEMENTED;
}
int cf_push(CfRing *q, const CfValue *v, unsigned n, CfHook h, void *c) {
  (void)q;
  (void)v;
  (void)n;
  (void)h;
  (void)c;
  return CF_UNIMPLEMENTED;
}
int cf_pop(CfRing *q, CfValue *v, unsigned n, CfHook h, void *c) {
  (void)q;
  (void)v;
  (void)n;
  (void)h;
  (void)c;
  return CF_UNIMPLEMENTED;
}
int cf_increment(atomic_uint *v, unsigned *o, unsigned *a, unsigned s) {
  (void)v;
  (void)o;
  (void)a;
  (void)s;
  return CF_UNIMPLEMENTED;
}
int cf_pool_init(CfPool *p, unsigned l) {
  (void)p;
  (void)l;
  return CF_UNIMPLEMENTED;
}
int cf_take(CfPool *p, unsigned w, unsigned s, CfTicket *o, unsigned *n) {
  (void)p;
  (void)w;
  (void)s;
  (void)o;
  (void)n;
  return CF_UNIMPLEMENTED;
}
int cf_publish(CfPool *p, CfTicket t) {
  (void)p;
  (void)t;
  return CF_UNIMPLEMENTED;
}
int cf_cancel(CfPool *p, CfTicket t) {
  (void)p;
  (void)t;
  return CF_UNIMPLEMENTED;
}
int cf_retire(CfPool *p, CfTicket t) {
  (void)p;
  (void)t;
  return CF_UNIMPLEMENTED;
}
int cf_reclaim(CfPool *p, CfTicket t) {
  (void)p;
  (void)t;
  return CF_UNIMPLEMENTED;
}
