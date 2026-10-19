#include "collection.h"
/* Intentionally unfinished learner implementation. Never export assessor code.
 */
int vg_reset(Vg *g, unsigned n) {
  (void)g;
  (void)n;
  return VG_UNIMPLEMENTED;
}
int vg_bind(Vg *g, CvmValue v, VgRef *o) {
  (void)g;
  (void)v;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_pair(Vg *g, VgRef a, VgRef b, VgRef *o) {
  (void)g;
  (void)a;
  (void)b;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_field(Vg *g, VgRef r, unsigned n, VgRef *o) {
  (void)g;
  (void)r;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_store(Vg *g, VgRef r, unsigned n, VgRef v) {
  (void)g;
  (void)r;
  (void)n;
  (void)v;
  return VG_UNIMPLEMENTED;
}
int vg_acquire(Vg *g, unsigned c, VgRef r, VgLease *o) {
  (void)g;
  (void)c;
  (void)r;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_release(Vg *g, unsigned c, VgLease l) {
  (void)g;
  (void)c;
  (void)l;
  return VG_UNIMPLEMENTED;
}
int vg_weak_set(Vg *g, unsigned n, VgRef r) {
  (void)g;
  (void)n;
  (void)r;
  return VG_UNIMPLEMENTED;
}
int vg_weak_acquire(Vg *g, unsigned n, unsigned c, VgLease *l, VgRef *o) {
  (void)g;
  (void)n;
  (void)c;
  (void)l;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_begin(Vg *g) {
  (void)g;
  return VG_UNIMPLEMENTED;
}
int vg_step(Vg *g, unsigned n, unsigned *o) {
  (void)g;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_commit(Vg *g, uint16_t *o) {
  (void)g;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_transfer(Vg *a, Vg *b, VgRef r, unsigned n, VgRef *o) {
  (void)a;
  (void)b;
  (void)r;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
