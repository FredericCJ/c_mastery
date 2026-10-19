#include "service.h"
/* Implement each operation from CONTRACTS.md using the actual P3 producer. */
CsStatus cs_init(CsService *s,size_t n,unsigned g){(void)s;(void)n;(void)g;return CS_INVALID;}
CsStatus cs_offer(CsService *s,const CPMessage *m,int a,int r,int o){(void)s;(void)m;(void)a;(void)r;(void)o;return CS_INVALID;}
CsStatus cs_feed(CsService *s,unsigned i,const unsigned char *b,size_t n,size_t *u,int a,int r){(void)s;(void)i;(void)b;(void)n;(void)u;(void)a;(void)r;return CS_INVALID;}
CsStatus cs_begin(CsService *s){(void)s;return CS_INVALID;}
CsStatus cs_step(CsService *s,CsSink f,void *c){(void)s;(void)f;(void)c;return CS_INVALID;}
CsStatus cs_recover(CsService *s){(void)s;return CS_INVALID;}
CsStatus cs_lease(CsService *s,const unsigned char **b,size_t *n,unsigned *g){(void)s;(void)b;(void)n;(void)g;return CS_INVALID;}
CsStatus cs_complete(CsService *s,unsigned g,int q,size_t n,CsDone f,void *c){(void)s;(void)g;(void)q;(void)n;(void)f;(void)c;return CS_INVALID;}
CsStatus cs_close(CsService *s,int c){(void)s;(void)c;return CS_INVALID;}
int cs_conserved(const CsService *s){(void)s;return 0;}
