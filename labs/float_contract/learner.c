#include "float_contract.h"
/* Implement the contracts. FF_RANGE is an intentionally unfinished scaffold. */
FfStatus ff_sum(const double *x,size_t n,FfMethod m,double *o) {(void)x;(void)n;(void)m;(void)o;return FF_RANGE;}
FfStatus ff_dot(const double *a,const double *b,size_t n,double *o) {(void)a;(void)b;(void)n;(void)o;return FF_RANGE;}
FfStatus ff_compare(double a,double b,double x,double r,int z,int *o) {(void)a;(void)b;(void)x;(void)r;(void)z;(void)o;return FF_RANGE;}
FfStatus ff_ulps(double a,double b,uint64_t *o) {(void)a;(void)b;(void)o;return FF_RANGE;}
FfStatus ff_probe_ops(double a,double b,FfRound *o,const FfEnvOps *p) {(void)a;(void)b;(void)o;(void)p;return FF_RANGE;}
FfStatus ff_round_probe(double a,double b,FfRound *o) {(void)a;(void)b;(void)o;return FF_RANGE;}
