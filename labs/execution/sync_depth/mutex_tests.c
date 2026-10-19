#include "mutex.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "CDM_CHECK:%d:%s\n", __LINE__, #c);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int setup(cmx_kernel *k) {
  cmx_init(k, CMX_FIXED);
  for (unsigned i = 0; i < 4; i++)
    if (cmx_task_add(k, 9 - i, 0, 0x1000u + 0x200u * i, 256) != (int)i)
      return 0;
  return 1;
}
static int ownership(void) {
  cmx_kernel k;
  cmx_mutex a, b;
  cmx_cond c = {0};
  CHECK(setup(&k));
  CHECK(cmx_mutex_init(&k, &a, 0, 0) == CMX_OK);
  CHECK(cmx_mutex_init(&k, &b, 0, 0) == CMX_OK);
  CHECK(cmx_mutex_lock(&k, &a, 0, cmx_forever()) == CMX_OK);
  CHECK(cmx_mutex_unlock(&k, &a, 1) == CMX_NOT_OWNER && a.owner == 0);
  CHECK(cmx_mutex_lock(&k, &a, 0, cmx_forever()) == CMX_DEADLOCK);
  CHECK(cmx_exit(&k, 0, 0) == CMX_NOT_OWNER);
  CHECK(cmx_mutex_lock(&k, &b, 1, cmx_forever()) == CMX_OK);
  CHECK(cmx_mutex_lock(&k, &b, 0, cmx_forever()) == CMX_BLOCKED);
  unsigned sequence = k.sequence;
  CHECK(cmx_mutex_lock(&k, &a, 1, cmx_forever()) == CMX_DEADLOCK);
  CHECK(k.sequence == sequence && a.owner == 0 && b.owner == 1 &&
        k.task[0].state == CMX_WAITING && k.task[1].state == CMX_READY);
  CHECK(cmx_cancel(&k, 0) == CMX_OK && k.task[0].result == CMX_CLOSED);
  CHECK(cmx_mutex_unlock(&k, &b, 1) == CMX_OK);
  CHECK(cmx_mutex_close(&k, &a) == CMX_NOT_OWNER);
  CHECK(cmx_cond_wait(&k, &c, &a, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(a.owner < 0 && cmx_mutex_close(&k, &a) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k, &a, 2, cmx_forever()) == CMX_OK);
  CHECK(cmx_cancel(&k, 0) == CMX_OK && k.task[0].wait == CMX_W_MUTEX);
  CHECK(cmx_mutex_close(&k, &a) == CMX_NOT_OWNER);
  CHECK(cmx_mutex_unlock(&k, &a, 2) == CMX_OK && a.owner == 0 &&
        k.task[0].result == CMX_CLOSED);
  CHECK(cmx_mutex_unlock(&k, &a, 0) == CMX_OK);
  CHECK(cmx_mutex_close(&k, &a) == CMX_OK);
  CHECK(cmx_mutex_close(&k, &b) == CMX_OK && cmx_invariant(&k));
  /* Calling extension code prevents a missing extension from masquerading as
     a passing primitive-only test. */
  CDRecursive r;
  CHECK(cdm_recursive_init(&r, &k, 1) == CMX_OK);
  CHECK(cdm_recursive_lock(&r, 0, cmx_forever()) == CMX_OK);
  CHECK(cdm_recursive_lock(&r, 0, cmx_forever()) == CMX_FULL);
  CHECK(cdm_recursive_unlock(&r, 0) == CMX_OK &&
        cdm_recursive_close(&r) == CMX_OK);
  puts("CDM_OWNERSHIP:cycle-refused;condition-reference-retained;owner-exit-"
       "refused");
  return 0;
}
typedef struct {
  int owner, waiter;
  unsigned depth;
} Model;
static int step(Model *m, unsigned operation) {
  unsigned task = operation / 2;
  if (!(operation % 2)) {
    if (m->waiter == (int)task)
      return CMX_INVALID;
    if (m->owner == (int)task) {
      if (m->depth == 2)
        return CMX_FULL;
      ++m->depth;
      return CMX_OK;
    }
    if (m->owner < 0) {
      m->owner = (int)task;
      m->depth = 1;
      return CMX_OK;
    }
    m->waiter = (int)task;
    return CMX_BLOCKED;
  }
  if (m->owner != (int)task)
    return CMX_NOT_OWNER;
  if (--m->depth)
    return CMX_OK;
  m->owner = m->waiter;
  m->waiter = -1;
  if (m->owner >= 0)
    m->depth = 1;
  return CMX_OK;
}
static int recursive(void) {
  unsigned transitions = 0;
  for (unsigned code = 0; code < 4096; code++) {
    cmx_kernel k;
    CDRecursive r;
    Model m = {-1, -1, 0};
    unsigned word = code;
    CHECK(setup(&k));
    CHECK(cdm_recursive_init(&r, &k, 2) == CMX_OK);
    for (unsigned i = 0; i < 6; i++, word /= 4) {
      unsigned op = word % 4, task = op / 2;
      int want = step(&m, op);
      int got = (op % 2) ? cdm_recursive_unlock(&r, task)
                         : cdm_recursive_lock(&r, task, cmx_forever());
      CHECK(got == want && r.mutex.owner == m.owner && r.depth == m.depth);
      for (unsigned t = 0; t < 2; t++)
        CHECK((k.task[t].state == CMX_WAITING) == (m.waiter == (int)t));
      CHECK(cmx_invariant(&k));
      ++transitions;
    }
  }
  cmx_kernel k;
  CDRecursive r;
  CHECK(setup(&k));
  CHECK(cdm_recursive_init(&r, &k, UINT_MAX) == CMX_OK);
  CHECK(cdm_recursive_lock(&r, 0, cmx_forever()) == CMX_OK);
  r.depth =
      UINT_MAX - 1; /* Valid, directly constructed finite boundary fixture. */
  CHECK(cdm_recursive_lock(&r, 0, cmx_forever()) == CMX_OK &&
        r.depth == UINT_MAX);
  CHECK(cdm_recursive_lock(&r, 0, cmx_forever()) == CMX_FULL &&
        r.depth == UINT_MAX);
  CHECK(cdm_recursive_unlock(&r, 1) == CMX_NOT_OWNER && r.depth == UINT_MAX);
  CHECK(cdm_recursive_unlock(&r, UINT_MAX) == CMX_INVALID);
  CHECK(cdm_recursive_unlock(&r, 0) == CMX_OK && r.depth == UINT_MAX - 1);
  printf("CDM_RECURSIVE:histories=4096;transitions=%u;finite-depth=UINT_MAX\n",
         transitions);
  return 0;
}
typedef struct {
  cmx_kernel *k;
  cmx_mutex **locks;
  const unsigned *ranks;
  unsigned target, task;
  int result;
} Callback;
static void callback_lock(Callback *c) {
  c->result = cdm_ordered_lock(c->k, c->locks, c->ranks, 2, c->target, c->task,
                               cmx_forever());
}
static int order(void) {
  cmx_kernel k;
  cmx_mutex a, b;
  cmx_mutex *locks[] = {&a, &b};
  unsigned ranks[] = {10, 20};
  CHECK(setup(&k));
  CHECK(cmx_mutex_init(&k, &a, 0, 0) == CMX_OK);
  CHECK(cmx_mutex_init(&k, &b, 0, 0) == CMX_OK);
  CHECK(cdm_ordered_lock(&k, locks, ranks, 2, 1, 0, cmx_forever()) == CMX_OK);
  Callback c = {&k, locks, ranks, 0, 0, -1};
  callback_lock(&c);
  CHECK(c.result == CMX_DEADLOCK && b.owner == 0 && a.owner < 0);
  CHECK(cmx_mutex_unlock(&k, &b, 0) == CMX_OK);
  CHECK(cdm_ordered_lock(&k, locks, ranks, 2, 0, 0, cmx_forever()) == CMX_OK);
  c.target = 1;
  callback_lock(&c);
  CHECK(c.result == CMX_OK && a.owner == 0 && b.owner == 0);
  CHECK(cmx_mutex_lock(&k, &a, 1, cmx_at(k.now)) == CMX_TIMEOUT &&
        k.task[1].state == CMX_READY);
  CHECK(cmx_mutex_unlock(&k, &b, 0) == CMX_OK &&
        cmx_mutex_unlock(&k, &a, 0) == CMX_OK);
  cmx_kernel before = k;
  unsigned duplicate[] = {10, 10};
  CHECK(cdm_ordered_lock(&k, locks, duplicate, 2, 0, 0, cmx_forever()) ==
        CMX_INVALID);
  CHECK(!memcmp(&k, &before, sizeof k));
  CHECK(cdm_ordered_lock(&k, locks, ranks, 1, 0, 0, cmx_forever()) ==
        CMX_INVALID);
  CHECK(cdm_ordered_lock(&k, locks, ranks, 2, 0, UINT_MAX, cmx_forever()) ==
        CMX_INVALID);
  CHECK(cmx_invariant(&k));
  puts("CDM_ORDER:callback-reverse-refused;forward-admitted;complete-distinct-"
       "registry");
  return 0;
}
typedef struct {
  cmx_kernel *k;
  cmx_mutex *m;
  CDRecord *record;
  unsigned calls, observations;
  int lock_result, read_result;
} Observer;
static void observe(void *p) {
  Observer *o = p;
  CDRecord output = {123, 456};
  ++o->calls;
  o->lock_result = cmx_mutex_lock(o->k, o->m, 1, cmx_at(o->k->now));
  if (o->lock_result == CMX_OK) {
    ++o->observations;
    o->read_result = cdm_record_read(o->k, o->m, 1, o->record, &output);
    (void)cmx_mutex_unlock(o->k, o->m, 1);
  }
}
static int record(void) {
  cmx_kernel k;
  cmx_mutex m;
  CDRecord record, out;
  CHECK(setup(&k));
  CHECK(cmx_mutex_init(&k, &m, 0, 0) == CMX_OK);
  cdm_record_init(&record, 0);
  Observer o = {&k, &m, &record, 0, 0, -1, -1};
  for (unsigned i = 0; i < 64; i++) {
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    printf("OWNER:%u:acquire:0;balance=%d\n", i, record.balance);
    CHECK(cdm_record_apply(&k, &m, 0, &record, 1, observe, &o) == CMX_OK);
    CHECK(o.lock_result == CMX_TIMEOUT && !o.observations && m.owner == 0);
    CHECK(cmx_mutex_lock(&k, &m, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_mutex_unlock(&k, &m, 0) == CMX_OK && m.owner == 1);
    printf("OWNER:%u:handoff:1;balance=%d\n", i, record.balance);
    CHECK(cdm_record_read(&k, &m, 1, &record, &out) == CMX_OK &&
          out.balance == (int32_t)i + 1);
    CHECK(cmx_mutex_unlock(&k, &m, 1) == CMX_OK && m.owner < 0);
    printf("OWNER:%u:release:-1\n", i);
  }
  CHECK(o.calls == 64 && record.balance == 64 && cmx_invariant(&k));
  puts("CDM_RECORD:updates=64;owner-transitions=192;mixed-observations=0");
  return 0;
}
static int rollback(void) {
  cmx_kernel k;
  cmx_mutex m;
  CDRecord record, before, out = {321, 654};
  CHECK(setup(&k));
  CHECK(cmx_mutex_init(&k, &m, 0, 0) == CMX_OK);
  cdm_record_init(&record, INT32_MAX);
  before = record;
  CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
  CHECK(cdm_record_apply(&k, &m, 0, &record, 1, NULL, NULL) == CMX_FULL);
  CHECK(record.balance == before.balance &&
        record.checksum == before.checksum && m.owner == 0);
  CHECK(cdm_record_apply(&k, &m, 1, &record, -1, NULL, NULL) == CMX_NOT_OWNER);
  CHECK(cdm_record_read(&k, &m, 1, &record, &out) == CMX_NOT_OWNER &&
        out.balance == 321 && out.checksum == 654);
  CHECK(cdm_record_apply(&k, &m, 0, &record, INT32_MIN, NULL, NULL) == CMX_OK &&
        record.balance == -1);
  CHECK(cdm_record_apply(&k, &m, 0, &record, -INT32_MAX, NULL, NULL) ==
            CMX_OK &&
        record.balance == INT32_MIN);
  before = record;
  CHECK(cdm_record_apply(&k, &m, 0, &record, -1, NULL, NULL) == CMX_FULL);
  CHECK(record.balance == before.balance && record.checksum == before.checksum);
  CHECK(cmx_mutex_unlock(&k, &m, 0) == CMX_OK);
  puts("CDM_ROLLBACK:upper-lower-bound;unchanged-failure;owned-publication");
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "ownership"))
    return ownership();
  if (!strcmp(argv[1], "recursive"))
    return recursive();
  if (!strcmp(argv[1], "order"))
    return order();
  if (!strcmp(argv[1], "record"))
    return record();
  if (!strcmp(argv[1], "rollback"))
    return rollback();
  return 2;
}
