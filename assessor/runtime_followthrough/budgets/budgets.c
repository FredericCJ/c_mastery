#include "budgets.h"
#ifndef VL_MUTANT
#define VL_MUTANT 0
#endif
struct vg_before {
  uint32_t fuel, gc, host, allocations, collections;
};
static struct vg_before before(const struct cvm_cvm *v) {
  return (struct vg_before){v->fuel, v->gc_budget, v->host_budget, v->allocations,
                  v->collections};
}
static void report(const struct cvm_cvm *v, struct vg_before b, int status, struct vl_report *out) {
  struct vl_report r = {b.fuel - v->fuel,
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
int vl_run(struct cvm_cvm *v, const struct cvm_program *p, struct cvm_value *out, struct vl_report *ledger) {
  if (!v || !p || !out || !ledger)
    return CVM_RANGE;
  struct vg_before b = before(v);
  struct cvm_value value = cvm_int(0);
  int s = cvm_run(v, p, &value);
  report(v, b, s, ledger);
  if (s == CVM_OK || VL_MUTANT == 4)
    *out = value;
  return s;
}
int vl_step(struct vg_vs *s, struct cvm_value *out, struct vl_report *ledger) {
  if (!s || !out || !ledger)
    return CVM_RANGE;
  struct vg_before b = before(&s->vm);
  struct cvm_value value = cvm_int(0);
  int status = vs_step(s, &value);
  report(&s->vm, b, status, ledger);
  if (status == VS_DONE || VL_MUTANT == 4)
    *out = value;
  return status;
}
