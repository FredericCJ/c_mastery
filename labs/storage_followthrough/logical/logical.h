#ifndef STORAGE_FOLLOW_LOGICAL_H
#define STORAGE_FOLLOW_LOGICAL_H
#include "store.h"

enum
{
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

struct sl_receipt
{
    uint64_t id;
    int64_t total;
};

struct sl_counter
{
    store_io io;
    uint64_t id;
    int64_t total, delta;
    unsigned pending, poisoned;
};

/* SLR1: three exclusive pages, serialized live control; no external effects.
 * Exact sequential IDs within one image lifetime. Latest duplicate only;
 * older IDs are rejected, never reapplied. Outputs/control/backend disjoint.
 * Fresh constructor storage; callback borrow/reentry rules match store_io. */
int sl_format(store_io);
int sl_open(struct sl_counter *, store_io);
int sl_apply(struct sl_counter *, uint64_t id, int64_t delta,
             struct sl_receipt *);
int sl_get(const struct sl_counter *, struct sl_receipt *);
int sl_checkpoint(struct sl_counter *);
#endif
