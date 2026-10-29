#include "dag.h"
int dg_init(struct sa_dg *g,uint64_t w,const struct dg_spec *s){(void)g;(void)w;(void)s;return DG_INVALID;}
int dg_append(struct sa_dg *g,uint64_t p,uint64_t c,const unsigned *a,unsigned n,unsigned *o){(void)g;(void)p;(void)c;(void)a;(void)n;(void)o;return DG_INVALID;}
int dg_seal(struct sa_dg *g){(void)g;return DG_INVALID;}
int dg_claim(struct sa_dg *g,unsigned w,struct dg_claim *o){(void)g;(void)w;(void)o;return DG_INVALID;}
int dg_finish(struct sa_dg *g,struct dg_claim c,unsigned s,uint64_t r){(void)g;(void)c;(void)s;(void)r;return DG_INVALID;}
int dg_cancel(struct sa_dg *g,unsigned n){(void)g;(void)n;return DG_INVALID;}
int dg_snapshot(struct sa_dg *g,struct dg_snapshot *o){(void)g;(void)o;return DG_INVALID;}
int dg_drained(struct sa_dg *g){(void)g;return DG_INVALID;}
