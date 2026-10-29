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
enum ts_status {TS_OK,TS_EMPTY,TS_FULL,TS_RETRY,TS_INVALID,TS_UNSUPPORTED};
struct ts_node {atomic_uint state; unsigned next; uint32_t value;};
struct ts_stack {atomic_uint head,hazard[TS_THREADS];struct ts_node node[TS_NODES];unsigned capacity;};
/* Caller-owned, noncopied live authority; see CONTRACTS.md. */
struct ts_pop {struct ts_stack *owner;unsigned participant,node,next;uint32_t value;};
struct ts_retired {struct ts_stack *owner;unsigned node,state;};
enum ts_status ts_init(struct ts_stack *,unsigned capacity);
enum ts_status ts_push(struct ts_stack *,uint32_t);
enum ts_status ts_begin(struct ts_stack *,unsigned participant,struct ts_pop *fresh);
enum ts_status ts_commit(struct ts_pop *,uint32_t *out);
enum ts_status ts_cancel(struct ts_pop *);
enum ts_status ts_pop(struct ts_stack *,unsigned participant,uint32_t *out);
enum ts_status ts_scan(struct ts_stack *,unsigned node,struct ts_retired *fresh);
enum ts_status ts_reclaim(struct ts_retired *);
unsigned ts_collect(struct ts_stack *);
#endif
