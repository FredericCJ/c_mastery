#ifndef CORPUS_TREIBER_H
#define CORPUS_TREIBER_H
#include <stdatomic.h>
#include <stdint.h>
#include <limits.h>
#define TS_NODES 8u
#define TS_THREADS 4u
#define TS_NONE UINT_MAX
_Static_assert(UINT_MAX>=UINT32_MAX,"selected unsigned width at least32");
#ifndef TS_GEN_MAX
#define TS_GEN_MAX (UINT_MAX/4u)
#endif
_Static_assert(TS_GEN_MAX > 0 && TS_GEN_MAX <= UINT_MAX/4u,"generation extent");
typedef enum {TS_OK,TS_EMPTY,TS_FULL,TS_RETRY,TS_INVALID,TS_UNSUPPORTED} TsStatus;
typedef struct {atomic_uint state; unsigned next; uint32_t value;} TsNode;
typedef struct {atomic_uint head,hazard[TS_THREADS];TsNode node[TS_NODES];unsigned capacity;} TsStack;
/* Caller-owned, noncopied live authority; see CONTRACTS.md. */
typedef struct {TsStack *owner;unsigned participant,node,next;uint32_t value;} TsPop;
typedef struct {TsStack *owner;unsigned node,state;} TsRetired;
TsStatus ts_init(TsStack *,unsigned capacity);
TsStatus ts_push(TsStack *,uint32_t);
TsStatus ts_begin(TsStack *,unsigned participant,TsPop *fresh);
TsStatus ts_commit(TsPop *,uint32_t *out);
TsStatus ts_cancel(TsPop *);
TsStatus ts_pop(TsStack *,unsigned participant,uint32_t *out);
TsStatus ts_scan(TsStack *,unsigned node,TsRetired *fresh);
TsStatus ts_reclaim(TsRetired *);
unsigned ts_collect(TsStack *);
#endif
