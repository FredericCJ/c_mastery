#include "checkpoint_memory.h"
#include <limits.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
static int power2(size_t n) { return n && !(n&(n-1)); }
enum cpm_status cpm_arena_init(struct cpm_arena *a,unsigned char *bytes,size_t capacity,size_t provided,size_t required) {
    if(!a || (!bytes && capacity) || !power2(provided) || !power2(required) || required>256 || provided<required) return CPM_INVALID;
    a->bytes=bytes;a->capacity=capacity;a->used=0;a->alignment=required;a->generation=1;a->pins=0;return CPM_OK;
}
enum cpm_status cpm_reserve(struct cpm_arena *a,size_t n,struct cpm_span *out) {
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
enum cpm_status cpm_view(const struct cpm_arena *a,struct cpm_span span,const unsigned char **out) {
    if(!a || !out || !a->bytes || span.generation!=a->generation || span.offset>a->used || span.length>a->used-span.offset)return CPM_INVALID;
    *out=a->bytes+span.offset;return CPM_OK;
}
enum cpm_status cpm_pin(struct cpm_arena *a,struct cpm_span span) {
    const unsigned char *view;
    if(cpm_view(a,span,&view)!=CPM_OK || a->pins==UINT_MAX)return CPM_INVALID;
    ++a->pins;return CPM_OK;
}
enum cpm_status cpm_unpin(struct cpm_arena *a) { if(!a || !a->pins)return CPM_INVALID;--a->pins;return CPM_OK; }
enum cpm_status cpm_reset(struct cpm_arena *a) {
    if(!a)return CPM_INVALID;
    if(a->pins && CP_MUTANT!=7)return CPM_BUSY;
    if(a->generation==CPM_GENERATION_MAX)return CPM_FULL;
    ++a->generation;a->used=0;return CPM_OK;
}
void cpm_pool_init(struct cpm_pool *p) {
    for(unsigned i=0;i<CPM_SLOTS;++i){p->objects[i].value=0;p->state[i]=0;p->generation[i]=1;p->pins[i]=0;}
    p->attempts=0;p->fail_at=0;
}
void cpm_ring_init(struct cpm_ring *q) {q->head=0;q->count=0;for(unsigned i=0;i<CPM_SLOTS;++i){q->items[i].slot=0;q->items[i].generation=0;}}
void cpm_fail_at(struct cpm_pool *p,unsigned ordinal) {p->attempts=0;p->fail_at=ordinal;}
enum cpm_status cpm_allocate(struct cpm_pool *p,int value,struct cpm_handle *out) {
    if(!p || !out)return CPM_INVALID;
    if(p->attempts==UINT_MAX)return CPM_FULL;
    ++p->attempts;if(p->fail_at && p->attempts==p->fail_at)return CPM_FULL;
    for(unsigned i=0;i<CPM_SLOTS;++i) if(!p->state[i]) {
        p->objects[i].value=value;p->state[i]=1;out->slot=i;out->generation=p->generation[i];return CPM_OK;
    }
    return CPM_FULL;
}
static int valid(const struct cpm_pool *p,struct cpm_handle h) {return p && h.slot<CPM_SLOTS && p->state[h.slot]==1 && h.generation==p->generation[h.slot];}
enum cpm_status cpm_lookup(const struct cpm_pool *p,struct cpm_handle h,const struct cpm_record **out) {
    if(!out || !valid(p,h))return CPM_INVALID;
    *out=&p->objects[h.slot];return CPM_OK;
}
enum cpm_status cpm_pool_pin(struct cpm_pool *p,struct cpm_handle h) {if(!valid(p,h) || p->pins[h.slot]==UINT_MAX)return CPM_INVALID;++p->pins[h.slot];return CPM_OK;}
enum cpm_status cpm_pool_unpin(struct cpm_pool *p,struct cpm_handle h) {if(!valid(p,h) || !p->pins[h.slot])return CPM_INVALID;--p->pins[h.slot];return CPM_OK;}
enum cpm_status cpm_release(struct cpm_pool *p,struct cpm_handle h) {
    if(!valid(p,h))return CPM_INVALID;
    if(p->pins[h.slot])return CPM_BUSY;
    if(p->generation[h.slot]==CPM_GENERATION_MAX) {
        if(CP_MUTANT==8){p->state[h.slot]=0;p->generation[h.slot]=1;}
        else p->state[h.slot]=2;
    } else {p->state[h.slot]=0;++p->generation[h.slot];}
    return CPM_OK;
}
enum cpm_status cpm_prefix(struct cpm_pool *p,struct cpm_ring *q,const int *values,size_t n,size_t *accepted) {
    if(!p || !q || !accepted || (!values && n))return CPM_INVALID;
    *accepted=0;
    while(*accepted<n) {
        if(q->count==CPM_SLOTS)return CPM_FULL;
        struct cpm_handle h;enum cpm_status result=cpm_allocate(p,values[*accepted],&h);
        if(result!=CPM_OK)return result;
        q->items[(q->head+q->count)%CPM_SLOTS]=h;++q->count;++*accepted;
    }
    return CPM_OK;
}
enum cpm_status cpm_atomic(struct cpm_pool *p,struct cpm_ring *q,const int *values,size_t n) {
    if(!p || !q || (!values && n))return CPM_INVALID;
    if(n>CPM_SLOTS-q->count)return CPM_FULL;
    struct cpm_pool staged_pool=*p;struct cpm_ring staged_ring=*q;size_t accepted;
    enum cpm_status result=cpm_prefix(&staged_pool,&staged_ring,values,n,&accepted);
    if(result==CPM_OK || CP_MUTANT==6){*p=staged_pool;*q=staged_ring;}
    return result;
}
enum cpm_status cpm_pop(struct cpm_ring *q,struct cpm_handle *out) {
    if(!q || !out)return CPM_INVALID;
    if(!q->count)return CPM_FULL;
    *out=q->items[q->head];q->head=(q->head+1)%CPM_SLOTS;--q->count;return CPM_OK;
}
