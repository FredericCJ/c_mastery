#ifndef STORAGE_FOLLOW_TRANSACTIONS_H
#define STORAGE_FOLLOW_TRANSACTIONS_H
#include "store.h"
typedef struct StTxn {store *engine;store undo;uint64_t serial;int saved;} StTxn;
typedef struct {const StTxn *owner;uint64_t serial;} StSavepoint;
/* Initialize fresh, unowned stable-address wrapper over a valid inactive store.
   One externally serialized caller; no raw engine mutation while bound.
   Input spans and controls are live/disjoint; no escaped scratch borrows.
   Existing store IO context stays live and outside the copied workspace. */
int st_init(StTxn *,store *);
int st_begin(StTxn *);
int st_put(StTxn *,const void *,size_t,const void *,size_t);
int st_delete(StTxn *,const void *,size_t);
int st_savepoint(StTxn *,StSavepoint *);
int st_rollback(StTxn *,StSavepoint);
int st_release(StTxn *,StSavepoint);
int st_commit(StTxn *);
int st_abort(StTxn *);
#endif
