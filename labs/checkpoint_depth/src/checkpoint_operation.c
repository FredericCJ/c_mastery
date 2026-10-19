#include "checkpoint_operation.h"
CPOStatus cpo_init(CPOperation *o,unsigned n,void *c,CPOAllocate a,CPORelease f,CPOReport r){(void)o;(void)n;(void)c;(void)a;(void)f;(void)r;return CPO_ERROR;}
CPOStatus cpo_begin(CPOperation *o,const unsigned char *p,size_t n,unsigned *g){(void)o;(void)p;(void)n;(void)g;return CPO_ERROR;}
CPOStatus cpo_cancel(CPOperation *o,unsigned g){(void)o;(void)g;return CPO_ERROR;}
CPOStatus cpo_complete(CPOperation *o,unsigned g,int q){(void)o;(void)g;(void)q;return CPO_ERROR;}
CPOStatus cpo_destroy(CPOperation *o){(void)o;return CPO_ERROR;}
