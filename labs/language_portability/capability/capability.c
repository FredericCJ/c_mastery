#include "capability.h"
int lp_record(unsigned p,intmax_t v,struct lp_value *o) {(void)p;(void)v;(void)o;return 0;}
int lp_format(struct lp_value v,char *o,size_t n) {(void)v;(void)o;(void)n;return 0;}
int lp_transform(const int *restrict s,int *restrict d,size_t n) {(void)s;(void)d;(void)n;return 0;}
int lp_capability(unsigned r,unsigned o,unsigned *u) {(void)r;(void)o;(void)u;return 0;}
