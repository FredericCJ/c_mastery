#include "tdigest.h"
/* Learner task: complete bounded centroid construction, query and merge transfer. */
int td_init(Td*s,unsigned d){(void)s;(void)d;return TD_INVALID;}
int td_valid(const Td*s){(void)s;return 0;}
int td_add(Td*s,const double*v,size_t n){(void)s;(void)v;(void)n;return TD_INVALID;}
int td_merge(Td*s,const Td*t){(void)s;(void)t;return TD_INVALID;}
int td_recompress(Td*s,unsigned d){(void)s;(void)d;return TD_INVALID;}
int td_quantile(const Td*s,double q,double*v){(void)s;(void)q;(void)v;return TD_INVALID;}
int td_cdf(const Td*s,double x,double*v){(void)s;(void)x;(void)v;return TD_INVALID;}
