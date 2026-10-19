#ifndef STORAGE_FOLLOW_LOGICAL_H
#define STORAGE_FOLLOW_LOGICAL_H
#include "store.h"
enum {
  SL_OK = 0,
  SL_DUPLICATE,
  SL_INVALID,
  SL_STATE,
  SL_GAP,
  SL_STALE,
  SL_CONFLICT,
  SL_FULL,
  SL_IO,
  SL_UNCERTAIN,
  SL_CORRUPT,
  SL_UNIMPLEMENTED
};
typedef struct {
  uint64_t id;
  int64_t total;
} SlReceipt;
typedef struct {
  store_io io;
  uint64_t id;
  int64_t total, delta;
  unsigned pending, poisoned;
} SlCounter;
/* SLR1: three exclusive pages, serialized live control; no external effects.
 * Exact sequential IDs within one image lifetime. Latest duplicate only;
 * older IDs are rejected, never reapplied. Outputs/control/backend disjoint.
 * Fresh constructor storage; callback borrow/reentry rules match store_io. */
int sl_format(store_io);
int sl_open(SlCounter *, store_io);
int sl_apply(SlCounter *, uint64_t id, int64_t delta, SlReceipt *);
int sl_get(const SlCounter *, SlReceipt *);
int sl_checkpoint(SlCounter *);
#endif
