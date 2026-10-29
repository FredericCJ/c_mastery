#include "budgets.h"
/* Deliberately incomplete accounting adapter; import your actual VM providers.
 */
int vl_run(struct cvm_cvm *v, const struct cvm_program *p, struct cvm_value *o, struct vl_report *r) {
  (void)v;
  (void)p;
  (void)o;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
int vl_step(struct vg_vs *s, struct cvm_value *o, struct vl_report *r) {
  (void)s;
  (void)o;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
