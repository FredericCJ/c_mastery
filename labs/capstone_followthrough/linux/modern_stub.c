#include "modern.h"
er_progress cs_result_step(const unsigned char *b,size_t n,size_t i,CsResultSink *s){(void)b;(void)n;(void)s;return(er_progress){i,ER_NOT_IMPLEMENTED};}
void cs_slabs_init(CsSlabs *s){(void)s;}
CsStatus cs_slabs_share(CsSlabs *s,const CPMessage *m,CsSlabToken t[2]){(void)s;(void)m;(void)t;return CS_INVALID;}
CsStatus cs_slabs_view(CsSlabs *s,CsSlabToken t,const unsigned char **b,size_t *n){(void)s;(void)t;(void)b;(void)n;return CS_INVALID;}
CsStatus cs_slabs_drop(CsSlabs *s,CsSlabToken t){(void)s;(void)t;return CS_INVALID;}
CsStatus cs_receipt(CsReceipts *r,unsigned e,const CPMessage *m){(void)r;(void)e;(void)m;return CS_INVALID;}
int cs_resources_open(CsResources *r,int e,int i,int o,unsigned f,int m){(void)r;(void)e;(void)i;(void)o;(void)f;(void)m;return -1;}
int cs_resources_close(CsResources *r){(void)r;return -1;}
int cs_resources_borrow(CsResources *r){(void)r;return -1;}
int cs_resources_ack(CsResources *r){(void)r;return -1;}
