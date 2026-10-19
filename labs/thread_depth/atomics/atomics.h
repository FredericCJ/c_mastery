#ifndef THREAD_ATOMICS_H
#define THREAD_ATOMICS_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
/* Own enum, never rely on a compiler's memory_order integer encoding. */
enum TAOrder {
  TA_RELAXED,
  TA_CONSUME,
  TA_ACQUIRE,
  TA_RELEASE,
  TA_ACQ_REL,
  TA_SC
};
int ta_order_pair(unsigned success, unsigned failure);
enum TAResult { TA_SUCCESS, TA_LIMIT, TA_INVALID, TA_UNPROTECTED };
typedef int (*TACas)(void *, unsigned *, unsigned, int);
typedef struct {
  unsigned attempts, expected, desired, published;
} TAReport;
int ta_increment(TACas, void *, unsigned, unsigned, int, TAReport *);
typedef struct TANode {
  unsigned value;
} TANode;
typedef int (*TANodeCas)(void *, TANode **, TANode *, int);
typedef struct {
  unsigned attempts, published, derived_value;
} TANodeReport;
/* The explicit lease set is live until the operation completes. It is not
   acquired by pointer equality and is not a general reclamation mechanism. */
int ta_publish(TANodeCas, void *, TANode *, TANode *, TANode *const *, size_t,
               int, unsigned, TANodeReport *);
typedef struct {
  atomic_uint sequence;
  atomic_int a, b;
} TASnapshot;
void ta_snapshot_init(TASnapshot *);
int ta_snapshot_write(TASnapshot *, int);
int ta_snapshot_try(TASnapshot *, unsigned, int *);
typedef struct {
  atomic_uint pending;
  unsigned readers;
  int payload;
} TASlot;
int ta_slot_init(TASlot *, unsigned);
int ta_slot_publish(TASlot *, int);
int ta_slot_read(TASlot *, unsigned, int *);
/* Analysis-only access descriptor. No racy target expression is executed. */
int ta_data_race(unsigned same_location, unsigned first_writes,
                 unsigned second_writes, unsigned both_atomic,
                 unsigned happens_before);
/* Transitive closure for at most8 events. Each bit j of edges[i] is i->j. */
int ta_closure(unsigned *, size_t);
enum TALitmus { TA_MP, TA_SB };
enum TAModel { TA_MODEL_RELAXED, TA_MODEL_RA, TA_MODEL_SC, TA_MODEL_MIXED };
/* Restricted two-location, two-thread, constant-store model in CONTRACTS.md.
   Returns1 permitted,0 forbidden,-1 outside this model. */
int ta_litmus(unsigned, unsigned, unsigned, unsigned);
typedef struct {
  uint64_t first, second;
} TAPair;
typedef struct {
  atomic_uint value;
} TACounter;
void ta_counter_init(TACounter *, unsigned);
unsigned ta_counter_add(TACounter *, unsigned);
unsigned ta_counter_read(const TACounter *);
int ta_lockfree_admit(unsigned query, unsigned unresolved_helpers,
                      unsigned context_authorized);
int ta_dependency(unsigned pointer_value_used, unsigned control_only,
                  unsigned killed);
#endif
