#include "dag.h"
int dg_init(Dg *g,uint64_t w,const DgSpec *s){(void)g;(void)w;(void)s;return DG_INVALID;}
int dg_append(Dg *g,uint64_t p,uint64_t c,const unsigned *a,unsigned n,unsigned *o){(void)g;(void)p;(void)c;(void)a;(void)n;(void)o;return DG_INVALID;}
int dg_seal(Dg *g){(void)g;return DG_INVALID;}
int dg_claim(Dg *g,unsigned w,DgClaim *o){(void)g;(void)w;(void)o;return DG_INVALID;}
int dg_finish(Dg *g,DgClaim c,unsigned s,uint64_t r){(void)g;(void)c;(void)s;(void)r;return DG_INVALID;}
int dg_cancel(Dg *g,unsigned n){(void)g;(void)n;return DG_INVALID;}
int dg_snapshot(Dg *g,DgSnapshot *o){(void)g;(void)o;return DG_INVALID;}
int dg_drained(Dg *g){(void)g;return DG_INVALID;}
