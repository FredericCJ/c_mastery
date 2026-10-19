#include "memory.h"
#include <limits.h>
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
static int valid(CPXStore *s,CPXHandle h) {
 return s && s->records && h.slot<s->capacity && s->state[h.slot]==1 && s->generation[h.slot]==h.generation;
}
static void dispose(CPXStore *s) {
 if(s->nodes){s->backend.release(s->backend.context,s->nodes);s->nodes=NULL;}
 if(s->records){s->backend.release(s->backend.context,s->records);s->records=NULL;}
 s->cookie=0;s->count=0;s->head=s->tail=CPX_NONE;s->closing=2;
}
CPXStatus cpx_init(CPXStore *s,CPXBackend b,unsigned capacity,unsigned nodes) {
 if(!s || s->records || s->nodes || !b.allocate || !b.release || !b.subscribe || !b.unsubscribe || !capacity || capacity>CPX_LIMIT || !nodes || nodes>capacity)return CPX_INVALID;
 CPXStore next={0};next.backend=b;next.capacity=capacity;next.node_capacity=nodes;next.head=next.tail=CPX_NONE;
 next.records=b.allocate(b.context,capacity*sizeof *next.records);if(!next.records)return CPX_FULL;
 next.nodes=b.allocate(b.context,nodes*sizeof *next.nodes);if(!next.nodes){dispose(&next);return CPX_FULL;}
 for(unsigned i=0;i<capacity;++i){next.records[i]=(CPXRecord){0};next.generation[i]=1;}
 for(unsigned i=0;i<nodes;++i)next.nodes[i]=(CPXNode){{0,0},CPX_NONE,0};
 if(!b.subscribe(b.context,&next.cookie) || !next.cookie){dispose(&next);return CPX_FAILED;}
 *s=next;return CPX_OK;
}
CPXStatus cpx_close(CPXStore *s) {
 if(!s)return CPX_INVALID;
 if(!s->records)return CPX_OK;
 for(unsigned i=0;i<s->capacity;++i)if(s->pins[i])return CPX_BUSY;
 if(s->closing)return CPX_PENDING;
 s->closing=1;
 int done=s->backend.unsubscribe(s->backend.context,s->cookie);
 if(!done && CPX_MUTANT!=1)return CPX_PENDING;
 dispose(s);return CPX_OK;
}
CPXStatus cpx_acknowledge(CPXStore *s,uint64_t cookie,int quiescent) {
 if(!s || !s->records || s->closing!=1 || !cookie || cookie!=s->cookie)return CPX_INVALID;
 if(!quiescent)return CPX_PENDING;
 dispose(s);return CPX_OK;
}
CPXStatus cpx_allocate(CPXStore *s,const CPXRecord *record,CPXHandle *out) {
 if(!s || !s->records || !record || !out || record->length>16)return CPX_INVALID;
 if(s->closing)return CPX_BUSY;
 for(unsigned i=0;i<s->capacity;++i)if(!s->state[i]){
  s->records[i]=*record;s->state[i]=1;*out=(CPXHandle){i,s->generation[i]};return CPX_OK;
 }
 return CPX_FULL;
}
CPXStatus cpx_lookup(CPXStore *s,CPXHandle h,const CPXRecord **out) {
 if(!out || !valid(s,h))return CPX_INVALID;
 *out=&s->records[h.slot];return CPX_OK;
}
CPXStatus cpx_pin(CPXStore *s,CPXHandle h){if(!valid(s,h) || s->pins[h.slot]==UINT_MAX)return CPX_INVALID;++s->pins[h.slot];return CPX_OK;}
CPXStatus cpx_unpin(CPXStore *s,CPXHandle h){if(!valid(s,h) || !s->pins[h.slot])return CPX_INVALID;--s->pins[h.slot];return CPX_OK;}
CPXStatus cpx_release(CPXStore *s,CPXHandle h){
 if(!valid(s,h))return CPX_INVALID;
 if(s->pins[h.slot])return CPX_BUSY;
 if(s->generation[h.slot]==CPX_GENERATION_MAX)s->state[h.slot]=2;
 else {s->state[h.slot]=0;++s->generation[h.slot];}
 return CPX_OK;
}
CPXStatus cpx_relocate(CPXStore *s){
 if(!s || !s->records || s->closing)return CPX_INVALID;
 for(unsigned i=0;i<s->capacity;++i)if(s->pins[i])return CPX_BUSY;
 CPXRecord *next=s->backend.allocate(s->backend.context,s->capacity*sizeof *next);if(!next)return CPX_FULL;
 for(unsigned i=0;i<s->capacity;++i)next[i]=s->records[i];
 s->backend.release(s->backend.context,s->records);s->records=next;return CPX_OK;
}
CPXStatus cpx_grow(CPXStore *s,unsigned capacity){
 if(!s || !s->records || s->closing || capacity<=s->capacity || capacity>CPX_LIMIT)return CPX_INVALID;
 for(unsigned i=0;i<s->capacity;++i)if(s->pins[i])return CPX_BUSY;
 CPXRecord *records=s->backend.allocate(s->backend.context,capacity*sizeof *records);if(!records)return CPX_FULL;
 CPXNode *nodes=s->backend.allocate(s->backend.context,capacity*sizeof *nodes);
 if(!nodes){s->backend.release(s->backend.context,records);return CPX_FULL;}
 for(unsigned i=0;i<capacity;++i){records[i]=i<s->capacity?s->records[i]:(CPXRecord){0};nodes[i]=i<s->node_capacity?s->nodes[i]:(CPXNode){{0,0},CPX_NONE,0};}
 for(unsigned i=s->capacity;i<capacity;++i){s->state[i]=0;s->generation[i]=1;s->pins[i]=0;}
 s->backend.release(s->backend.context,s->nodes);s->backend.release(s->backend.context,s->records);
 s->records=records;s->nodes=nodes;s->capacity=s->node_capacity=capacity;return CPX_OK;
}
CPXStatus cpx_push(CPXStore *s,CPXHandle h){
 if(!valid(s,h))return CPX_INVALID;
 if(s->closing)return CPX_BUSY;
 if(s->node_attempts==UINT_MAX)return CPX_FULL;
 ++s->node_attempts;if(s->node_fail_at==s->node_attempts)return CPX_FULL;
 for(unsigned i=0;i<s->node_capacity;++i)if(!s->nodes[i].used){
  /* A queued node owns a pin; acquiring it precedes publishing the node. */
  if(cpx_pin(s,h)!=CPX_OK)return CPX_FULL;
  s->nodes[i]=(CPXNode){h,CPX_NONE,1};
  if(s->tail==CPX_NONE)s->head=i;else s->nodes[s->tail].next=i;
  s->tail=i;++s->count;return CPX_OK;
 }
 return CPX_FULL;
}
CPXStatus cpx_pop(CPXStore *s,CPXHandle *out){
 if(!s || !s->records || !out)return CPX_INVALID;
 if(!s->count)return CPX_FULL;
 unsigned node=s->head;*out=s->nodes[node].handle;s->head=s->nodes[node].next;
 s->nodes[node].used=0;if(!--s->count)s->tail=CPX_NONE;
 return cpx_unpin(s,*out);
}
CPXStatus cpx_batch(CPXStore *s,const CPXRecord *records,size_t n,size_t *accepted){
 if(!s || !accepted || (!records && n))return CPX_INVALID;
 *accepted=0;
 while(*accepted<n){
  CPXHandle h;CPXStatus status=cpx_allocate(s,&records[*accepted],&h);if(status!=CPX_OK)return status;
  status=cpx_push(s,h);if(status!=CPX_OK){if(CPX_MUTANT!=2)(void)cpx_release(s,h);return status;}
  ++*accepted;
 }
 return CPX_OK;
}
CPXStatus cpx_lease_begin(CPXLease *l,CPMArena *a,CPMSpan span,uint64_t cookie){
 if(!l || l->pending || !cookie || cpm_pin(a,span)!=CPM_OK)return CPX_INVALID;
 *l=(CPXLease){a,cookie,1,0};return CPX_OK;
}
CPXStatus cpx_lease_cancel(CPXLease *l){if(!l || !l->pending)return CPX_INVALID;l->cancelled=1;return CPX_PENDING;}
CPXStatus cpx_lease_complete(CPXLease *l,uint64_t cookie,int quiescent){
 if(!l || !l->pending || l->cookie!=cookie)return CPX_INVALID;
 if(!quiescent)return CPX_PENDING;
 if(cpm_unpin(l->arena)!=CPM_OK)return CPX_INVALID;
 l->pending=0;return CPX_OK;
}
