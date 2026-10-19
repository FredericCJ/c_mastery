#include "collection.h"
#ifdef VG_STANDALONE
int printf(const char *, ...);
int puts(const char *);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
#else
#include <stdio.h>
#include <string.h>
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("VG_CHECK %u %s\n", (unsigned)__LINE__, #x);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static Vg a, b;
static Cvm base, before;
static int same_value(CvmValue x, CvmValue y) {
  return x.tag == y.tag && x.integer == y.integer &&
         x.generation == y.generation;
}
/* Compare semantic fields: C structure padding has no portable golden bytes. */
static int same_vm(const Cvm *x, const Cvm *y) {
  if (x->sp != y->sp || x->depth != y->depth ||
      x->root_count != y->root_count || x->arena != y->arena ||
      x->fuel != y->fuel || x->gc_budget != y->gc_budget ||
      x->host_budget != y->host_budget || x->allocations != y->allocations ||
      x->collections != y->collections || x->host != y->host ||
      x->host_context != y->host_context)
    return 0;
  for (unsigned i = 0; i < CVM_STACK; i++)
    if (!same_value(x->stack[i], y->stack[i]))
      return 0;
  for (unsigned i = 0; i < CVM_ROOTS; i++)
    if (!same_value(x->roots[i], y->roots[i]))
      return 0;
  for (unsigned i = 0; i < CVM_FRAMES; i++) {
    const CvmFrame *p = &x->frames[i], *q = &y->frames[i];
    if (p->function != q->function || p->ip != q->ip || p->base != q->base)
      return 0;
    for (unsigned j = 0; j < CVM_LOCALS; j++)
      if (p->initialized[j] != q->initialized[j] ||
          !same_value(p->locals[j], q->locals[j]))
        return 0;
  }
  for (unsigned i = 0; i < CVM_HEAP; i++) {
    const CvmObject *p = &x->heap[i], *q = &y->heap[i];
    if (p->used != q->used || p->marked != q->marked ||
        p->generation != q->generation || !same_value(p->left, q->left) ||
        !same_value(p->right, q->right))
      return 0;
  }
  return 1;
}
static VgRef integer(Vg *g, int32_t n) {
  VgRef r = {g, g->epoch, {CVM_INT, n, 0}};
  return r;
}
static int collect(Vg *g, uint16_t *dead) {
  unsigned scanned, total = 0;
  int status;
  CHECK(vg_begin(g) == VG_OK);
  status = vg_step(g, 0, &scanned);
  CHECK((status == VG_OK || status == VG_DONE) && scanned == 0);
  do {
    status = vg_step(g, 1, &scanned);
    CHECK(scanned <= 1);
    total += scanned;
    CHECK(total <= 16);
  } while (status == VG_OK);
  CHECK(status == VG_DONE);
  CHECK(vg_commit(g, dead) == VG_OK);
  return 0;
}
static int identities(void) {
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  CHECK(vg_reset(&a, 1) == VG_OK && vg_reset(&b, 1) == VG_OK);
  VgRef p, q, out;
  CHECK(vg_pair(&a, integer(&a, 7), integer(&a, 8), &p) == VG_OK);
  CHECK(vg_field(&b, p, 0, &out) == VG_STALE);
  CHECK(vg_reset(&a, 1) == VG_OK);
  CHECK(vg_pair(&a, integer(&a, 9), integer(&a, 10), &q) == VG_OK);
  CHECK(p.value.integer == q.value.integer &&
        p.value.generation == q.value.generation);
  CHECK(vg_field(&a, p, 0, &out) == VG_STALE);
  CHECK(vg_field(&a, q, 0, &out) == VG_OK && out.value.integer == 9);
  a.epoch = UINT32_MAX;
  CHECK(vg_reset(&a, 1) == VG_STATE);
  memset(&a, 0, sizeof a);
  CHECK(vg_reset(&a, 0) == VG_OK);
  CHECK(vg_pair(&a, integer(&a, 1), integer(&a, 2), &p) == VG_OK);
  uint16_t dead;
  CHECK(collect(&a, &dead) == 0 && dead == 1);
  CHECK(vg_pair(&a, integer(&a, 3), integer(&a, 4), &q) == VG_OK);
  CHECK(p.value.integer == q.value.integer &&
        p.value.generation != q.value.generation);
  CHECK(vg_field(&a, p, 0, &out) == VG_STALE);
  CHECK(collect(&a, &dead) == 0 && dead == 1);
  a.vm.heap[0].generation = UINT32_MAX - 1;
  CHECK(vg_pair(&a, integer(&a, 0), integer(&a, 0), &p) == VG_OK);
  CHECK(p.value.integer == 0 && p.value.generation == UINT32_MAX);
  CHECK(collect(&a, &dead) == 0 && dead == 1);
  CHECK(vg_pair(&a, integer(&a, 0), integer(&a, 0), &q) == VG_OK &&
        q.value.integer == 1);
  CHECK(vg_field(&a, p, 0, &out) == VG_STALE);
  VgLease first, second;
  CHECK(vg_acquire(&a, 1, q, &first) == VG_OK);
  CHECK(vg_acquire(&a, 2, q, &second) == VG_OK);
  CHECK(vg_release(&a, 2, first) == VG_STALE);
  CHECK(vg_reset(&a, 0) == VG_STATE);
  CHECK(vg_release(&a, 1, first) == VG_OK);
  CHECK(vg_release(&a, 1, first) == VG_STALE);
  CHECK(collect(&a, &dead) == 0 && dead == 0);
  CHECK(vg_release(&a, 2, second) == VG_OK);
  a.registrations.registrations.generation[0] = UINT32_MAX;
  CHECK(vg_acquire(&a, 1, q, &first) == VG_OK);
  CHECK(vg_release(&a, 1, first) == VG_OK);
  CHECK(a.registrations.registrations.state[0] == 2);
  CHECK(vg_acquire(&a, 1, q, &second) == VG_OK && (uint32_t)second.ticket == 1);
  CHECK(vg_release(&a, 1, second) == VG_OK);
  CHECK(vg_reset(&a, 0) == VG_OK);
  CHECK(vg_release(&a, 1, second) == VG_STALE);
  puts("VG_IDENTITY_PASS reset_alias_last_generation_two_clients");
  return 0;
}
static int barriers(void) {
  memset(&a, 0, sizeof a);
  CHECK(vg_reset(&a, 0) == VG_OK);
  VgRef x, y, z;
  VgLease lx, ly;
  uint16_t dead;
  unsigned n;
  CHECK(vg_pair(&a, integer(&a, 1), integer(&a, 2), &x) == VG_OK);
  CHECK(vg_pair(&a, integer(&a, 3), integer(&a, 4), &y) == VG_OK);
  CHECK(vg_acquire(&a, 1, x, &lx) == VG_OK);
  CHECK(vg_begin(&a) == VG_OK);
  CHECK(vg_commit(&a, &dead) == VG_STATE);
  CHECK(vg_pair(&a, x, y, &z) == VG_STATE);
  CHECK(vg_step(&a, 16, &n) == VG_DONE && n == 1);
  CHECK(vg_store(&a, x, 0, y) == VG_OK);
  CHECK(vg_step(&a, 16, &n) == VG_DONE);
  CHECK(vg_commit(&a, &dead) == VG_OK && dead == 0);
  CHECK(vg_field(&a, x, 0, &z) == VG_OK && z.value.integer == y.value.integer);
  CHECK(vg_store(&a, x, 0, integer(&a, 0)) == VG_OK);
  CHECK(vg_begin(&a) == VG_OK && vg_step(&a, 16, &n) == VG_DONE);
  CHECK(vg_acquire(&a, 2, y, &ly) == VG_OK);
  CHECK(vg_step(&a, 16, &n) == VG_DONE);
  CHECK(vg_commit(&a, &dead) == VG_OK && a.vm.heap[y.value.integer].used);
  CHECK(vg_release(&a, 2, ly) == VG_OK);
  CHECK(vg_weak_set(&a, 0, y) == VG_OK);
  CHECK(vg_weak_set(&a, 1, x) == VG_OK);
  CHECK(collect(&a, &dead) == 0);
  CHECK(dead == (uint16_t)(1u << y.value.integer));
  CHECK(vg_weak_acquire(&a, 0, 2, &ly, &z) == VG_STALE);
  CHECK(vg_weak_acquire(&a, 1, 2, &ly, &z) == VG_OK);
  CHECK(vg_release(&a, 1, lx) == VG_OK);
  CHECK(collect(&a, &dead) == 0 && dead == 0);
  CHECK(vg_release(&a, 2, ly) == VG_OK);
  CHECK(collect(&a, &dead) == 0 && !a.weak_present[1]);
  puts("VG_BARRIER_PASS late_store_late_root_weak_upgrade");
  return 0;
}
static int allocation(void) {
  memset(&a, 0, sizeof a);
  CHECK(vg_reset(&a, 0) == VG_OK);
  VgRef p[16], parent, out;
  for (unsigned i = 0; i < 16; i++)
    CHECK(vg_pair(&a, integer(&a, (int32_t)i), integer(&a, 0), &p[i]) == VG_OK);
  CHECK(vg_pair(&a, p[0], p[1], &parent) == VG_OK);
  CHECK(vg_field(&a, p[0], 0, &out) == VG_OK && out.value.integer == 0);
  CHECK(vg_field(&a, p[1], 0, &out) == VG_OK && out.value.integer == 1);
  CHECK(vg_field(&a, parent, 1, &out) == VG_OK &&
        out.value.integer == p[1].value.integer);
  unsigned occupied = 0;
  for (unsigned i = 0; i < 16; i++)
    occupied += a.vm.heap[i].used;
  CHECK(occupied == 3 && a.commits == 1 && a.temporary_count == 0);
  memset(&a, 0, sizeof a);
  CHECK(vg_reset(&a, 1) == VG_OK);
  for (unsigned i = 0; i < 16; i++)
    CHECK(vg_pair(&a, integer(&a, 0), integer(&a, 0), &p[i]) == VG_OK);
  out = integer(&a, 91);
  CHECK(vg_pair(&a, p[0], p[1], &out) == VG_FULL && out.value.integer == 91);
  a.vm.allocations = UINT32_MAX;
  CHECK(vg_pair(&a, p[0], p[1], &out) == VG_FULL);
  puts("VG_ALLOCATION_PASS arena_vs_reclaim_both_temporaries");
  return 0;
}
static int transfer(void) {
  memset(&a, 0, sizeof a);
  memset(&b, 0, sizeof b);
  CHECK(vg_reset(&a, 1) == VG_OK && vg_reset(&b, 1) == VG_OK);
  VgRef x, y, z, out, copy, field;
  CHECK(vg_pair(&a, integer(&a, 11), integer(&a, 12), &x) == VG_OK);
  CHECK(vg_pair(&a, x, x, &y) == VG_OK);
  CHECK(vg_store(&a, x, 1, y) == VG_OK);
  CHECK(vg_pair(&a, integer(&a, 99), integer(&a, 0), &z) == VG_OK);
  CHECK(vg_pair(&b, integer(&b, 77), integer(&b, 88), &z) == VG_OK);
  b.vm.heap[1].generation = UINT32_MAX;
  base = a.vm;
  before = b.vm;
  for (unsigned cut = 1; cut <= 7; cut++) {
    out = integer(&b, 91);
    CHECK(vg_transfer(&a, &b, y, cut, &out) == VG_FULL);
    CHECK(same_vm(&before, &b.vm) && out.value.integer == 91);
    CHECK(same_vm(&base, &a.vm));
  }
  CHECK(vg_transfer(&a, &b, y, 8, &out) == VG_BAD);
  CHECK(vg_transfer(&a, &b, y, 0, &copy) == VG_OK);
  CHECK(same_vm(&base, &a.vm));
  CHECK(copy.value.integer == 2);
  CHECK(vg_field(&b, copy, 0, &field) == VG_OK && field.value.integer == 3);
  CHECK(vg_field(&b, copy, 1, &out) == VG_OK &&
        out.value.integer == field.value.integer);
  CHECK(vg_field(&b, field, 1, &out) == VG_OK &&
        out.value.integer == copy.value.integer);
  CHECK(vg_field(&b, z, 0, &out) == VG_OK && out.value.integer == 77);
  CHECK(vg_field(&a, x, 0, &out) == VG_OK && out.value.integer == 11);
  CHECK(vg_reset(&a, 1) == VG_OK && vg_field(&a, x, 0, &out) == VG_STALE);
  CHECK(vg_field(&b, field, 0, &out) == VG_OK && out.value.integer == 11);
  CHECK(vg_pair(&a, integer(&a, 1), integer(&a, 2), &x) == VG_OK);
  for (unsigned i = 4; i < 16; i++)
    CHECK(vg_pair(&b, integer(&b, 0), integer(&b, 0), &out) == VG_OK);
  before = b.vm;
  out = integer(&b, 91);
  CHECK(vg_transfer(&a, &b, x, 0, &out) == VG_FULL && same_vm(&before, &b.vm) &&
        out.value.integer == 91);
  CHECK(vg_transfer(&a, &b, integer(&a, 42), 1, &out) == VG_FULL &&
        same_vm(&before, &b.vm) && out.value.integer == 91);
  CHECK(vg_transfer(&a, &b, integer(&a, 42), 0, &out) == VG_OK &&
        out.value.integer == 42 && same_vm(&before, &b.vm));
  puts("VG_TRANSFER_PASS cycles_aliases_seven_atomic_fault_cuts");
  return 0;
}
static int baseline(void) {
  CvmValue p[16], out;
  cvm_init(&base, 0, UINT32_MAX, 0);
  for (unsigned i = 0; i < 16; i++)
    CHECK(cvm_pair(&base, cvm_int((int32_t)i), cvm_int(0), &p[i]) == CVM_OK);
  base.heap[0].right = p[1];
  base.heap[1].right = p[0];
  CHECK(cvm_root_push(&base, p[0]) == CVM_OK);
  base.stack[base.sp++] = p[2];
  base.depth = 1;
  base.frames[0].locals[0] = p[3];
  base.frames[0].initialized[0] = 1;
  base.frames[0].locals[1] = p[4]; /* uninitialized slot is not a root */
  before = base;
  /* 3 examined roots + 2 fields of each of 4 live cells + 16 sweep cells. */
  for (unsigned budget = 0; budget < 27; budget++) {
    base = before;
    base.gc_budget = budget;
    CHECK(cvm_collect(&base) == CVM_GC_BUDGET);
    for (unsigned i = 0; i < 16; i++)
      CHECK(base.heap[i].used == before.heap[i].used &&
            base.heap[i].generation == before.heap[i].generation);
    CHECK(base.collections == 0);
    base.gc_budget = 27;
    CHECK(cvm_collect(&base) == CVM_OK && base.gc_budget == 0);
    for (unsigned i = 0; i < 16; i++)
      CHECK(base.heap[i].used == (i < 4));
  }
  cvm_init(&base, 0, UINT32_MAX, 0);
  for (unsigned i = 0; i < 16; i++)
    CHECK(cvm_pair(&base, cvm_int((int32_t)i), cvm_int(0), &p[i]) == CVM_OK);
  for (unsigned i = 0; i < 7; i++)
    CHECK(cvm_root_push(&base, cvm_int(0)) == CVM_OK);
  out = cvm_int(91);
  before = base;
  CHECK(cvm_pair(&base, p[0], p[1], &out) == CVM_STACK_LIMIT &&
        base.root_count == 7 && out.integer == 91);
  CHECK(memcmp(base.heap, before.heap, sizeof base.heap) == 0 &&
        base.collections == 0);
  base.root_count = 0;
  CHECK(cvm_pair(&base, p[0], p[1], &out) == CVM_OK && base.root_count == 0);
  CHECK(cvm_field(&base, p[0], 0, &p[2]) == CVM_OK && p[2].integer == 0);
  CHECK(cvm_field(&base, p[1], 0, &p[2]) == CVM_OK && p[2].integer == 1);
  cvm_init(&base, 0, UINT32_MAX, 0);
  for (unsigned i = 0; i < 16; i++)
    CHECK(cvm_pair(&base, cvm_int((int32_t)i), cvm_int(0), &p[i]) == CVM_OK);
  /* A helper allocation occurs after evaluating left, before right exists. */
  CHECK(cvm_root_push(&base, p[0]) == CVM_OK);
  CHECK(cvm_pair(&base, cvm_int(50), cvm_int(51), &out) == CVM_OK);
  CHECK(cvm_field(&base, p[0], 0, &p[2]) == CVM_OK && p[2].integer == 0);
  CHECK(cvm_pair(&base, p[0], out, &p[3]) == CVM_OK);
  cvm_root_pop(&base);
  CHECK(base.root_count == 0);
  /* Deliberately omit the C temporary root: a local property violation, not
   * a native C use-after-free or a claim of code execution. */
  cvm_init(&base, 0, UINT32_MAX, 0);
  for (unsigned i = 0; i < 16; i++)
    CHECK(cvm_pair(&base, cvm_int((int32_t)i), cvm_int(0), &p[i]) == CVM_OK);
  CHECK(cvm_pair(&base, cvm_int(50), cvm_int(51), &out) == CVM_OK);
  CHECK(cvm_field(&base, p[0], 0, &p[2]) == CVM_STALE);
  cvm_init(&base, 0, UINT32_MAX, 0);
  base.arena = 1;
  for (unsigned i = 0; i < 16; i++)
    CHECK(cvm_pair(&base, cvm_int((int32_t)i), cvm_int(0), &p[i]) == CVM_OK);
  CHECK(cvm_pair(&base, cvm_int(0), cvm_int(0), &out) == CVM_HEAP_LIMIT);
  /* Explicit stopped raw-VM policy transition; no controller API is bypassed.
   */
  base.arena = 0;
  CHECK(cvm_pair(&base, cvm_int(0), cvm_int(0), &out) == CVM_OK);
  unsigned occupied = 0;
  for (unsigned i = 0; i < 16; i++)
    occupied += base.heap[i].used;
  CHECK(occupied == 1 && base.collections == 1);
  CHECK(cvm_field(&base, p[15], 0, &out) == CVM_STALE);
  puts("VG_BASELINE_PASS roots_cycles_27_budget_cuts_second_push_helper");
  return 0;
}
int main(void) {
  if (identities() || barriers() || allocation() || transfer() || baseline())
    return 1;
  puts("VG_PASS actual_CVM_ORVM_incremental_weak_phase_transfer");
  return 0;
}
