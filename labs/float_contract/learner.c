#include "float_contract.h"
/* Implement the contracts. FF_RANGE is an intentionally unfinished scaffold. */
enum ff_status ff_sum(const double *x,size_t n,enum ff_method m,double *o) {(void)x;(void)n;(void)m;(void)o;return FF_RANGE;}
enum ff_status ff_dot(const double *a,const double *b,size_t n,double *o) {(void)a;(void)b;(void)n;(void)o;return FF_RANGE;}
enum ff_status ff_compare(double a,double b,double x,double r,int z,int *o) {(void)a;(void)b;(void)x;(void)r;(void)z;(void)o;return FF_RANGE;}
enum ff_status ff_ulps(double a,double b,uint64_t *o) {(void)a;(void)b;(void)o;return FF_RANGE;}
enum ff_status ff_probe_ops(double a,double b,struct ff_round *o,const struct ff_env_ops *p) {(void)a;(void)b;(void)o;(void)p;return FF_RANGE;}
enum ff_status ff_round_probe(double a,double b,struct ff_round *o) {(void)a;(void)b;(void)o;return FF_RANGE;}
