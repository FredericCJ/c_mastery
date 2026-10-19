#include "atomics.h"
int ta_lockfree_admit(unsigned a, unsigned b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int ta_litmus(unsigned a, unsigned b, unsigned c, unsigned d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
void ta_counter_init(TACounter *p, unsigned n) { atomic_init(&p->value, n); }
unsigned ta_counter_add(TACounter *p, unsigned n) {
  (void)p;
  (void)n;
  return 0;
}
unsigned ta_counter_read(const TACounter *p) {
  (void)p;
  return 0;
}
int ta_dependency(unsigned a, unsigned b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
void ta_snapshot_init(TASnapshot *p) { (void)p; }
int ta_snapshot_write(TASnapshot *p, int n) {
  (void)p;
  (void)n;
  return TA_INVALID;
}
int ta_snapshot_try(TASnapshot *p, unsigned n, int *out) {
  (void)p;
  (void)n;
  (void)out;
  return TA_INVALID;
}
int ta_slot_init(TASlot *p, unsigned n) {
  (void)p;
  (void)n;
  return TA_INVALID;
}
int ta_slot_publish(TASlot *p, int n) {
  (void)p;
  (void)n;
  return TA_INVALID;
}
int ta_slot_read(TASlot *p, unsigned n, int *out) {
  (void)p;
  (void)n;
  (void)out;
  return TA_INVALID;
}
int ta_data_race(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return -1;
}
int ta_closure(unsigned *p, size_t n) {
  (void)p;
  (void)n;
  return TA_INVALID;
}
int ta_order_pair(unsigned success, unsigned failure) {
  (void)success;
  (void)failure;
  return 0;
}
int ta_increment(TACas operation, void *context, unsigned initial,
                 unsigned limit, int one_attempt, TAReport *out) {
  (void)operation;
  (void)context;
  (void)initial;
  (void)limit;
  (void)one_attempt;
  (void)out;
  return TA_INVALID;
}
int ta_publish(TANodeCas operation, void *context, TANode *owned,
               TANode *expected, TANode *const *leases, size_t count,
               int derive, unsigned limit, TANodeReport *out) {
  (void)operation;
  (void)context;
  (void)owned;
  (void)expected;
  (void)leases;
  (void)count;
  (void)derive;
  (void)limit;
  (void)out;
  return TA_INVALID;
}
