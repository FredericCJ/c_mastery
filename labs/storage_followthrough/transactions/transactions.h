#ifndef STORAGE_FOLLOW_TRANSACTIONS_H
#define STORAGE_FOLLOW_TRANSACTIONS_H
#include "store.h"

struct st_txn
{
    store *engine;
    store undo;
    uint64_t serial;
    int saved;
};

struct st_savepoint
{
    const struct st_txn *owner;
    uint64_t serial;
};

/* Initialize fresh, unowned stable-address wrapper over a valid inactive store.
   One externally serialized caller; no raw engine mutation while bound.
   Input spans and controls are live/disjoint; no escaped scratch borrows.
   Existing store IO context stays live and outside the copied workspace. */
int st_init(struct st_txn *, store *);
int st_begin(struct st_txn *);
int st_put(struct st_txn *, const void *, size_t, const void *, size_t);
int st_delete(struct st_txn *, const void *, size_t);
int st_savepoint(struct st_txn *, struct st_savepoint *);
int st_rollback(struct st_txn *, struct st_savepoint);
int st_release(struct st_txn *, struct st_savepoint);
int st_commit(struct st_txn *);
int st_abort(struct st_txn *);
#endif
