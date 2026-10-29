#include "collection.h"
/* Intentionally unfinished learner implementation. Never export assessor code.
 */
int vg_reset(struct vg_vg *g, unsigned n) {
  (void)g;
  (void)n;
  return VG_UNIMPLEMENTED;
}
int vg_bind(struct vg_vg *g, struct cvm_value v, struct vg_ref *o) {
  (void)g;
  (void)v;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_pair(struct vg_vg *g, struct vg_ref a, struct vg_ref b, struct vg_ref *o) {
  (void)g;
  (void)a;
  (void)b;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_field(struct vg_vg *g, struct vg_ref r, unsigned n, struct vg_ref *o) {
  (void)g;
  (void)r;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_store(struct vg_vg *g, struct vg_ref r, unsigned n, struct vg_ref v) {
  (void)g;
  (void)r;
  (void)n;
  (void)v;
  return VG_UNIMPLEMENTED;
}
int vg_acquire(struct vg_vg *g, unsigned c, struct vg_ref r, struct vg_lease *o) {
  (void)g;
  (void)c;
  (void)r;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_release(struct vg_vg *g, unsigned c, struct vg_lease l) {
  (void)g;
  (void)c;
  (void)l;
  return VG_UNIMPLEMENTED;
}
int vg_weak_set(struct vg_vg *g, unsigned n, struct vg_ref r) {
  (void)g;
  (void)n;
  (void)r;
  return VG_UNIMPLEMENTED;
}
int vg_weak_acquire(struct vg_vg *g, unsigned n, unsigned c, struct vg_lease *l, struct vg_ref *o) {
  (void)g;
  (void)n;
  (void)c;
  (void)l;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_begin(struct vg_vg *g) {
  (void)g;
  return VG_UNIMPLEMENTED;
}
int vg_step(struct vg_vg *g, unsigned n, unsigned *o) {
  (void)g;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_commit(struct vg_vg *g, uint16_t *o) {
  (void)g;
  (void)o;
  return VG_UNIMPLEMENTED;
}
int vg_transfer(struct vg_vg *a, struct vg_vg *b, struct vg_ref r, unsigned n, struct vg_ref *o) {
  (void)a;
  (void)b;
  (void)r;
  (void)n;
  (void)o;
  return VG_UNIMPLEMENTED;
}
