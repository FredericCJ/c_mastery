#include "budgets.h"
#ifndef VL_MUTANT
#define VL_MUTANT 0
#endif
typedef struct {
  uint32_t fuel, gc, host, allocations, collections;
} Before;
static Before before(const Cvm *v) {
  return (Before){v->fuel, v->gc_budget, v->host_budget, v->allocations,
                  v->collections};
}
static void report(const Cvm *v, Before b, int status, VlReport *out) {
  VlReport r = {b.fuel - v->fuel,
                b.gc - v->gc_budget,
                b.host - v->host_budget,
                v->allocations - b.allocations,
                v->collections - b.collections,
                status};
  if (VL_MUTANT == 1) {
    uint32_t x = r.fuel;
    r.fuel = r.gc;
    r.gc = x;
  }
  if (VL_MUTANT == 2 && status != CVM_OK && status != VS_DONE && r.fuel)
    r.fuel--;
  if (VL_MUTANT == 3 && status != CVM_OK && status != VS_DONE)
    r.host = 0;
  if (VL_MUTANT == 5)
    r.gc = 16 * r.collections;
  *out = r;
}
int vl_run(Cvm *v, const CvmProgram *p, CvmValue *out, VlReport *ledger) {
  if (!v || !p || !out || !ledger)
    return CVM_RANGE;
  Before b = before(v);
  CvmValue value = cvm_int(0);
  int s = cvm_run(v, p, &value);
  report(v, b, s, ledger);
  if (s == CVM_OK || VL_MUTANT == 4)
    *out = value;
  return s;
}
int vl_step(Vs *s, CvmValue *out, VlReport *ledger) {
  if (!s || !out || !ledger)
    return CVM_RANGE;
  Before b = before(&s->vm);
  CvmValue value = cvm_int(0);
  int status = vs_step(s, &value);
  report(&s->vm, b, status, ledger);
  if (status == VS_DONE || VL_MUTANT == 4)
    *out = value;
  return status;
}
