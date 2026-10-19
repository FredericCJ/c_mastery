#ifndef STORAGE_FOLLOW_DURABILITY_GUARD_H
#define STORAGE_FOLLOW_DURABILITY_GUARD_H
#include "store.h"
/* Called after the complete raw redo/checksum/generation pass and before any
 * home write. Source/commit/backend remain exclusively owned and unchanged
 * except for reads. Validate complete CST2 structural authority through the
 * actual imported decoder/checker, without recursively opening/recovering. */
int dg_validate(store *,const uint8_t commit[4096]);
#endif
