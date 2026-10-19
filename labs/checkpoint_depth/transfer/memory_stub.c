#include "memory.h"
/* Learner implementation: this file intentionally does not complete the task. */
CPXStatus cpx_init(CPXStore *s,CPXBackend b,unsigned c,unsigned n){(void)s;(void)b;(void)c;(void)n;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_close(CPXStore *s){(void)s;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_acknowledge(CPXStore *s,uint64_t c,int q){(void)s;(void)c;(void)q;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_allocate(CPXStore *s,const CPXRecord *r,CPXHandle *h){(void)s;(void)r;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_lookup(CPXStore *s,CPXHandle h,const CPXRecord **r){(void)s;(void)h;(void)r;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_pin(CPXStore *s,CPXHandle h){(void)s;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_unpin(CPXStore *s,CPXHandle h){(void)s;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_release(CPXStore *s,CPXHandle h){(void)s;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_relocate(CPXStore *s){(void)s;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_grow(CPXStore *s,unsigned c){(void)s;(void)c;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_push(CPXStore *s,CPXHandle h){(void)s;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_pop(CPXStore *s,CPXHandle *h){(void)s;(void)h;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_batch(CPXStore *s,const CPXRecord *r,size_t n,size_t *a){(void)s;(void)r;(void)n;(void)a;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_lease_begin(CPXLease *l,CPMArena *a,CPMSpan s,uint64_t c){(void)l;(void)a;(void)s;(void)c;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_lease_cancel(CPXLease *l){(void)l;return CPX_NOT_IMPLEMENTED;}
CPXStatus cpx_lease_complete(CPXLease *l,uint64_t c,int q){(void)l;(void)c;(void)q;return CPX_NOT_IMPLEMENTED;}
