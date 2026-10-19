#ifndef STORAGE_FOLLOW_REFS_H
#define STORAGE_FOLLOW_REFS_H
#include "tree.h"
typedef struct {store *owner;uint32_t id;uint64_t incarnation;} StoreRef;
/* Observational references can be minted/read only outside a transaction.
 * They do not pin pages. The exact store owner is live, stable and noncopied;
 * constructor reuse of owner storage or switching images requires retiring all
 * prior references. All calls are externally serialized. Output is disjoint.
 * Staged references never escape: aborted unpublished generations may roll back.
 * STORE_STALE preserves output after release/reuse; generation MAX retires a
 * freed slot while allocation continues at later eligible slots. */
int store_ref_make(store *,uint32_t,StoreRef *);
int store_ref_read(StoreRef,store_node *);
#endif
