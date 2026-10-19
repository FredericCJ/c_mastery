#include "scope.h"
int rzf_scope_create(RzfScope *p,uint64_t n,uint64_t b,RzfAllocator a,RzfScope **o){(void)p;(void)n;(void)b;(void)a;(void)o;return RZ_INVALID;}
int rzf_scope_retain(RzfScope *s,RzfScope **o){(void)s;(void)o;return RZ_INVALID;}
void rzf_scope_release(RzfScope **s){(void)s;}
int rzf_borrow_begin(RzfBorrow *b,RzfScope *s){(void)b;(void)s;return RZ_INVALID;}
int rzf_borrow_ack(RzfBorrow *b){(void)b;return RZ_INVALID;}
int rzf_budget_export(const RzfScope *s,uint64_t n,uint64_t t,RzfBudget *o){(void)s;(void)n;(void)t;(void)o;return RZ_INVALID;}
int rzf_budget_import(const RzfBudget *b,RzfScope *p,uint64_t n,uint64_t e,RzfAllocator a,RzfScope **o){(void)b;(void)p;(void)n;(void)e;(void)a;(void)o;return RZ_INVALID;}
int rzf_budget_encode(const RzfBudget *b,unsigned char o[16]){(void)b;(void)o;return RZ_INVALID;}
int rzf_budget_decode(const unsigned char w[16],RzfBudget *o){(void)w;(void)o;return RZ_INVALID;}
