#ifndef STORAGE_FOLLOW_WORKFLOW_H
#define STORAGE_FOLLOW_WORKFLOW_H
#include "services.h"
enum {
  SW_PROGRESS = 0,
  SW_DONE,
  SW_COMPENSATED,
  SW_INVALID,
  SW_CONFLICT,
  SW_RECOVER,
  SW_UNIMPLEMENTED
};
enum { SW_PREPARED = 1, SW_DEBITED, SW_COMPENSATING, SW_FINISHED, SW_REVERSED };
struct sw_flow {
  store *journal;
  struct vg_vh *debit, *credit;
  int32_t amount;
  unsigned decline, phase, poisoned;
  int last_store_result;
};
/* One original workflow per three exclusive images; no participant activity
 * outside this coordinator, except read-only queries. Vh mode VERSIONED5,
 * service3 enabled, matching initial no-receipt participant state. Constructor
 * writes the intent before effects. Open reuses an existing intent after all
 * poisoned stores have been reopened. Controls/configuration live and stable.
 * Decline is a durable pre-admission policy: credit is never submitted on that
 * branch. No compensation is inferred from an uncertain credit response. */
int sw_create(struct sw_flow *, store *journal, struct vg_vh *debit, struct vg_vh *credit, int32_t amount,
              unsigned decline);
int sw_open(struct sw_flow *, store *journal, struct vg_vh *debit, struct vg_vh *credit);
int sw_step(struct sw_flow *);
#endif
