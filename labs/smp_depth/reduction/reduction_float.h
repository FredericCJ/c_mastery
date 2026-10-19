#ifndef SMP_REDUCTION_FLOAT_H
#define SMP_REDUCTION_FLOAT_H
#include "reduction.h"
typedef struct {double sum,correction;} SrFloat;
/* Hosted binary64 experiment only. Finite input and finite intermediate sums;
 * default round-to-nearest, no fast-math/reassociation. Neumaier compensation
 * is an accuracy experiment, not an exact-sum guarantee for arbitrary data. */
int sr_float_chunk(const double *,size_t,unsigned workers,unsigned worker,int compensated,SrFloat *);
int sr_float_join(const SrFloat *,unsigned workers,int compensated,double *);
#endif
