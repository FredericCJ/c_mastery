#include "tdigest.h"
/* Learner task: complete bounded centroid construction, query and merge transfer. */
int td_init(struct td_td*s,unsigned d){(void)s;(void)d;return TD_INVALID;}
int td_valid(const struct td_td*s){(void)s;return 0;}
int td_add(struct td_td*s,const double*v,size_t n){(void)s;(void)v;(void)n;return TD_INVALID;}
int td_merge(struct td_td*s,const struct td_td*t){(void)s;(void)t;return TD_INVALID;}
int td_recompress(struct td_td*s,unsigned d){(void)s;(void)d;return TD_INVALID;}
int td_quantile(const struct td_td*s,double q,double*v){(void)s;(void)q;(void)v;return TD_INVALID;}
int td_cdf(const struct td_td*s,double x,double*v){(void)s;(void)x;(void)v;return TD_INVALID;}
