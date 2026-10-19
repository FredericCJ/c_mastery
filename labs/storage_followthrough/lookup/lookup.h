#ifndef STORAGE_FOLLOW_LOOKUP_H
#define STORAGE_FOLLOW_LOOKUP_H
#include "store.h"
#define SL_RECORDS (31u*7u)
typedef struct {uint8_t klen,vlen,key[16],value[64];} SlRecord;
typedef struct {SlRecord records[SL_RECORDS];size_t count;uint64_t generation;int from_transaction,ready;} SlSnapshot;
typedef struct {store *engine;} SlText;
/* Printable ASCII bytes0x20..0x7e, length1..16. Only A..Z folds to a..z.
   Output16bytes and length are disjoint; input may overlap output bytes. */
int sl_normalize(const uint8_t *,size_t,uint8_t out[16],size_t *length);
/* Bind only a store whose existing keys are canonical. After binding every
   mutation must use this normalization policy; raw mixed-policy writes violate
   the caller contract. No automatic locale/Unicode normalization is promised. */
int sl_text_bind(store *,SlText *);
int sl_text_put(SlText *,const void *,size_t,const void *,size_t);
int sl_text_get(SlText *,const void *,size_t,void *,size_t,size_t *);
int sl_text_scan(SlText *,const void *,size_t,store_visit,void *);
/* Capture performs no client callbacks; full structural check precedes scan.
   Output disjoint from store/backend. After capture, owned copied records are
   independent of source lifetime. Live immutable snapshot/out count/context are
   disjoint; callback borrows one record for its call only. Callback may reenter
   the source store, but may not mutate/destroy this snapshot or retain the borrow. */
int sl_snapshot_capture(store *,SlSnapshot *);
typedef int (*SlVisit)(void *,const SlRecord *);
/* Nonzero callback result is a normal stop. calls counts actual invocations,
   not accepted records. budget0 performs no callbacks. */
int sl_snapshot_scan(const SlSnapshot *,const void *,size_t,size_t budget,SlVisit,void *,size_t *calls);
#endif
