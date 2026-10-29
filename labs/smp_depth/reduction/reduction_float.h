#ifndef SMP_REDUCTION_FLOAT_H
#define SMP_REDUCTION_FLOAT_H
#include "reduction.h"
struct sr_float {double sum,correction;};
/* Hosted binary64 experiment only. Finite input and finite intermediate sums;
 * default round-to-nearest, no fast-math/reassociation. Neumaier compensation
 * is an accuracy experiment, not an exact-sum guarantee for arbitrary data. */
int sr_float_chunk(const double *,size_t,unsigned workers,unsigned worker,int compensated,struct sr_float *);
int sr_float_join(const struct sr_float *,unsigned workers,int compensated,double *);
#endif
