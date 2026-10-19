#include "checkpoint_memory.h"
#include <limits.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
static int power2(size_t n) { return n && !(n&(n-1)); }
CPMStatus cpm_arena_init(CPMArena *a,unsigned char *bytes,size_t capacity,size_t provided,size_t required) {
    if(!a || (!bytes && capacity) || !power2(provided) || !power2(required) || required>256 || provided<required) return CPM_INVALID;
    a->bytes=bytes;a->capacity=capacity;a->used=0;a->alignment=required;a->generation=1;a->pins=0;return CPM_OK;
}
CPMStatus cpm_reserve(CPMArena *a,size_t n,CPMSpan *out) {
    if(!a || !out || n>65535)return CPM_INVALID;
    if(a->capacity-a->used<3)return CPM_FULL;
    size_t start=a->used+3;
    size_t padding=(a->alignment-start%a->alignment)%a->alignment;
    if(padding>a->capacity-start)return CPM_FULL;
    start+=padding;
    if(n>a->capacity-start) {
        if(CP_MUTANT==5)a->used=start;
        return CPM_FULL;
    }
    a->bytes[a->used]=(unsigned char)n;a->bytes[a->used+1]=(unsigned char)(n>>8);a->bytes[a->used+2]=0;
    out->offset=start;out->length=n;out->generation=a->generation;a->used=start+n;return CPM_OK;
}
CPMStatus cpm_view(const CPMArena *a,CPMSpan span,const unsigned char **out) {
    if(!a || !out || !a->bytes || span.generation!=a->generation || span.offset>a->used || span.length>a->used-span.offset)return CPM_INVALID;
    *out=a->bytes+span.offset;return CPM_OK;
}
CPMStatus cpm_pin(CPMArena *a,CPMSpan span) {
    const unsigned char *view;
    if(cpm_view(a,span,&view)!=CPM_OK || a->pins==UINT_MAX)return CPM_INVALID;
    ++a->pins;return CPM_OK;
}
CPMStatus cpm_unpin(CPMArena *a) { if(!a || !a->pins)return CPM_INVALID;--a->pins;return CPM_OK; }
CPMStatus cpm_reset(CPMArena *a) {
    if(!a)return CPM_INVALID;
    if(a->pins && CP_MUTANT!=7)return CPM_BUSY;
    if(a->generation==CPM_GENERATION_MAX)return CPM_FULL;
    ++a->generation;a->used=0;return CPM_OK;
}
void cpm_pool_init(CPMPool *p) {
    for(unsigned i=0;i<CPM_SLOTS;++i){p->objects[i].value=0;p->state[i]=0;p->generation[i]=1;p->pins[i]=0;}
    p->attempts=0;p->fail_at=0;
}
void cpm_ring_init(CPMRing *q) {q->head=0;q->count=0;for(unsigned i=0;i<CPM_SLOTS;++i){q->items[i].slot=0;q->items[i].generation=0;}}
void cpm_fail_at(CPMPool *p,unsigned ordinal) {p->attempts=0;p->fail_at=ordinal;}
CPMStatus cpm_allocate(CPMPool *p,int value,CPMHandle *out) {
    if(!p || !out)return CPM_INVALID;
    if(p->attempts==UINT_MAX)return CPM_FULL;
    ++p->attempts;if(p->fail_at && p->attempts==p->fail_at)return CPM_FULL;
    for(unsigned i=0;i<CPM_SLOTS;++i) if(!p->state[i]) {
        p->objects[i].value=value;p->state[i]=1;out->slot=i;out->generation=p->generation[i];return CPM_OK;
    }
    return CPM_FULL;
}
static int valid(const CPMPool *p,CPMHandle h) {return p && h.slot<CPM_SLOTS && p->state[h.slot]==1 && h.generation==p->generation[h.slot];}
CPMStatus cpm_lookup(const CPMPool *p,CPMHandle h,const CPMRecord **out) {
    if(!out || !valid(p,h))return CPM_INVALID;
    *out=&p->objects[h.slot];return CPM_OK;
}
CPMStatus cpm_pool_pin(CPMPool *p,CPMHandle h) {if(!valid(p,h) || p->pins[h.slot]==UINT_MAX)return CPM_INVALID;++p->pins[h.slot];return CPM_OK;}
CPMStatus cpm_pool_unpin(CPMPool *p,CPMHandle h) {if(!valid(p,h) || !p->pins[h.slot])return CPM_INVALID;--p->pins[h.slot];return CPM_OK;}
CPMStatus cpm_release(CPMPool *p,CPMHandle h) {
    if(!valid(p,h))return CPM_INVALID;
    if(p->pins[h.slot])return CPM_BUSY;
    if(p->generation[h.slot]==CPM_GENERATION_MAX) {
        if(CP_MUTANT==8){p->state[h.slot]=0;p->generation[h.slot]=1;}
        else p->state[h.slot]=2;
    } else {p->state[h.slot]=0;++p->generation[h.slot];}
    return CPM_OK;
}
CPMStatus cpm_prefix(CPMPool *p,CPMRing *q,const int *values,size_t n,size_t *accepted) {
    if(!p || !q || !accepted || (!values && n))return CPM_INVALID;
    *accepted=0;
    while(*accepted<n) {
        if(q->count==CPM_SLOTS)return CPM_FULL;
        CPMHandle h;CPMStatus result=cpm_allocate(p,values[*accepted],&h);
        if(result!=CPM_OK)return result;
        q->items[(q->head+q->count)%CPM_SLOTS]=h;++q->count;++*accepted;
    }
    return CPM_OK;
}
CPMStatus cpm_atomic(CPMPool *p,CPMRing *q,const int *values,size_t n) {
    if(!p || !q || (!values && n))return CPM_INVALID;
    if(n>CPM_SLOTS-q->count)return CPM_FULL;
    CPMPool staged_pool=*p;CPMRing staged_ring=*q;size_t accepted;
    CPMStatus result=cpm_prefix(&staged_pool,&staged_ring,values,n,&accepted);
    if(result==CPM_OK || CP_MUTANT==6){*p=staged_pool;*q=staged_ring;}
    return result;
}
CPMStatus cpm_pop(CPMRing *q,CPMHandle *out) {
    if(!q || !out)return CPM_INVALID;
    if(!q->count)return CPM_FULL;
    *out=q->items[q->head];q->head=(q->head+1)%CPM_SLOTS;--q->count;return CPM_OK;
}
