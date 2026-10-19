/* Intentionally unsolved: preserve learner work separately from assessor code. */
#include "checkpoint_memory.h"
CPMStatus cpm_arena_init(CPMArena*a,unsigned char*b,size_t c,size_t p,size_t r){(void)a;(void)b;(void)c;(void)p;(void)r;return CPM_INVALID;}
CPMStatus cpm_reserve(CPMArena*a,size_t n,CPMSpan*s){(void)a;(void)n;(void)s;return CPM_INVALID;}
CPMStatus cpm_view(const CPMArena*a,CPMSpan s,const unsigned char**o){(void)a;(void)s;(void)o;return CPM_INVALID;}
CPMStatus cpm_pin(CPMArena*a,CPMSpan s){(void)a;(void)s;return CPM_INVALID;}
CPMStatus cpm_unpin(CPMArena*a){(void)a;return CPM_INVALID;}
CPMStatus cpm_reset(CPMArena*a){(void)a;return CPM_INVALID;}
void cpm_pool_init(CPMPool*p){(void)p;}
void cpm_ring_init(CPMRing*q){(void)q;}
void cpm_fail_at(CPMPool*p,unsigned o){(void)p;(void)o;}
CPMStatus cpm_allocate(CPMPool*p,int v,CPMHandle*h){(void)p;(void)v;(void)h;return CPM_INVALID;}
CPMStatus cpm_lookup(const CPMPool*p,CPMHandle h,const CPMRecord**o){(void)p;(void)h;(void)o;return CPM_INVALID;}
CPMStatus cpm_pool_pin(CPMPool*p,CPMHandle h){(void)p;(void)h;return CPM_INVALID;}
CPMStatus cpm_pool_unpin(CPMPool*p,CPMHandle h){(void)p;(void)h;return CPM_INVALID;}
CPMStatus cpm_release(CPMPool*p,CPMHandle h){(void)p;(void)h;return CPM_INVALID;}
CPMStatus cpm_prefix(CPMPool*p,CPMRing*q,const int*v,size_t n,size_t*a){(void)p;(void)q;(void)v;(void)n;(void)a;return CPM_INVALID;}
CPMStatus cpm_atomic(CPMPool*p,CPMRing*q,const int*v,size_t n){(void)p;(void)q;(void)v;(void)n;return CPM_INVALID;}
CPMStatus cpm_pop(CPMRing*q,CPMHandle*h){(void)q;(void)h;return CPM_INVALID;}
