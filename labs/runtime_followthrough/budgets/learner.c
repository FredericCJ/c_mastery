#include "budgets.h"
/* Deliberately incomplete accounting adapter; import your actual VM providers.
 */
int vl_run(Cvm *v, const CvmProgram *p, CvmValue *o, VlReport *r) {
  (void)v;
  (void)p;
  (void)o;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
int vl_step(Vs *s, CvmValue *o, VlReport *r) {
  (void)s;
  (void)o;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
