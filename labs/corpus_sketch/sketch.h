#ifndef CORPUS_SKETCH_H
#define CORPUS_SKETCH_H
#include <stdint.h>
#include <stddef.h>
/* All pointers name live disjoint complete objects, except exact self-merge.
   Objects are initialized through the API. Callers serialize access. */
enum { SK_OK=0, SK_INVALID=1, SK_OVERFLOW=2 };
typedef struct { unsigned p,depth; uint32_t mask[4][5]; unsigned bias[4]; } SkCmConfig;
typedef struct { SkCmConfig config; uint64_t total,cell[4][32]; } SkCm;
int sk_cm_init(SkCm *,const SkCmConfig *);
int sk_cm_add(SkCm *,uint32_t key,uint64_t amount);
int sk_cm_query(const SkCm *,uint32_t key,uint64_t *);
int sk_cm_merge(SkCm *,const SkCm *);
typedef struct { unsigned p; uint32_t identity; uint8_t reg[256]; } SkHll;
int sk_hll_init(SkHll *,unsigned p,uint32_t hash_identity);
int sk_hll_add(SkHll *,uint32_t hash);
int sk_hll_estimate(const SkHll *,double *);
int sk_hll_merge(SkHll *,const SkHll *);
/* Merge a finer sketch into an equal/coarser destination; preserve hash bits. */
int sk_hll_fold(SkHll *,const SkHll *);
typedef struct { unsigned universe,count; uint8_t rank[120][16]; } SkMhConfig;
typedef struct { SkMhConfig config; uint8_t minimum[120]; } SkMh;
typedef struct { unsigned numerator,denominator; } SkFraction;
int sk_mh_init(SkMh *,const SkMhConfig *);
int sk_mh_add(SkMh *,unsigned element);
int sk_mh_similarity(const SkMh *,const SkMh *,SkFraction *);
int sk_mh_merge(SkMh *,const SkMh *);
/* Count complete matching bands, not a Jaccard estimate. */
int sk_mh_bands(const SkMh *,const SkMh *,unsigned rows,unsigned *matching);
#endif
