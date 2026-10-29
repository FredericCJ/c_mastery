#include "scope.h"
int rzf_scope_create(struct rzf_scope *p,uint64_t n,uint64_t b,struct rzf_allocator a,struct rzf_scope **o){(void)p;(void)n;(void)b;(void)a;(void)o;return RZ_INVALID;}
int rzf_scope_retain(struct rzf_scope *s,struct rzf_scope **o){(void)s;(void)o;return RZ_INVALID;}
void rzf_scope_release(struct rzf_scope **s){(void)s;}
int rzf_borrow_begin(struct rzf_borrow *b,struct rzf_scope *s){(void)b;(void)s;return RZ_INVALID;}
int rzf_borrow_ack(struct rzf_borrow *b){(void)b;return RZ_INVALID;}
int rzf_budget_export(const struct rzf_scope *s,uint64_t n,uint64_t t,struct rzf_budget *o){(void)s;(void)n;(void)t;(void)o;return RZ_INVALID;}
int rzf_budget_import(const struct rzf_budget *b,struct rzf_scope *p,uint64_t n,uint64_t e,struct rzf_allocator a,struct rzf_scope **o){(void)b;(void)p;(void)n;(void)e;(void)a;(void)o;return RZ_INVALID;}
int rzf_budget_encode(const struct rzf_budget *b,unsigned char o[16]){(void)b;(void)o;return RZ_INVALID;}
int rzf_budget_decode(const unsigned char w[16],struct rzf_budget *o){(void)w;(void)o;return RZ_INVALID;}
