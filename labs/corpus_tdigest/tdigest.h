#ifndef CORPUS_TDIGEST_H
#define CORPUS_TDIGEST_H
#include <stdint.h>
#include <stddef.h>
enum { TD_OK=0,TD_INVALID=1,TD_CAPACITY=2 };
typedef struct { double mean; uint32_t weight; } TdCentroid;
typedef struct { unsigned compression,used; uint32_t count; TdCentroid centroids[128]; } Td;
/* C17 hosted double, finite samples in [-1e9,1e9], total<=65536.
   Live complete disjoint objects; exact self-merge admitted; caller serializes. */
int td_init(Td *,unsigned compression); /* 8..32 */
int td_valid(const Td *);
int td_add(Td *,const double *values,size_t count); /* batch <=64, zero admits NULL */
int td_merge(Td *,const Td *); /* destination compression <= source */
int td_recompress(Td *,unsigned compression); /* only equal/coarser */
int td_quantile(const Td *,double q,double *); /* generalized inverse, q in[0,1] */
int td_cdf(const Td *,double x,double *); /* right-continuous fitted distribution */
#endif
