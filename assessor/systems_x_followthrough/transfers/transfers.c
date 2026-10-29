#include "transfers.h"
#ifndef SX_MUTANT
#define SX_MUTANT 0
#endif
int sx_remap(struct sx_map*m,unsigned p,unsigned present,unsigned user,unsigned writable,unsigned char fill){
 if(!m||p>=4||present>1||user>1||writable>1)return B5_INVALID;
 if(m->version[p]>=3)return B5_RANGE;
 m->version[p]++;m->pages[p].present=present;m->pages[p].user=user;m->pages[p].writable=writable;
 for(unsigned i=0;i<16;i++)m->pages[p].bytes[i]=fill;
 return B5_OK;
}
int sx_copy(struct sx_map*m,size_t address,const unsigned char*input,size_t n,sx_between_fn between,void*ctx){
 struct b5_page staged[4];unsigned versions[4];int rc;
 if(!m)return B5_INVALID;
 for(unsigned i=0;i<4;i++){staged[i]=m->pages[i];versions[i]=m->version[i];}
 rc=b5_copy_to_user(staged,address,input,n);if(rc!=B5_OK)return rc;
 if(n&&between)between(m,ctx);
#if SX_MUTANT != 1
 for(size_t i=0;i<n;i++)if(m->version[(address+i)/16]!=versions[(address+i)/16])return B5_CONFLICT;
#else
 (void)versions;
#endif
 for(size_t i=0;i<n;i++)m->pages[(address+i)/16].bytes[(address+i)%16]=staged[(address+i)/16].bytes[(address+i)%16];
 return B5_OK;
}
int sx_partial(struct sx_map*m,size_t address,const unsigned char*input,size_t n,struct sx_copy_result*out){
 struct sx_copy_result result={0,B5_OK};
 if(!m||!out||(!input&&n))return B5_INVALID;
 if(address>64||n>64-address){*out=(struct sx_copy_result){0,B5_RANGE};return B5_OK;}
 for(size_t i=0;i<n;i++){int rc=b5_copy_to_user(m->pages,address+i,input+i,1);if(rc!=B5_OK){result.status=rc;break;}result.copied++;}
#if SX_MUTANT == 2
 if(result.status!=B5_OK)result.copied=0;
#endif
 *out=result;return B5_OK;
}
static int valid(const struct sx_ledger*l,struct sx_alias a,unsigned actor){return l&&a.slot<4&&actor<2&&l->ledger.slots[a.slot].live&&l->ledger.slots[a.slot].identity==a.identity&&l->generation[a.slot]==a.generation&&l->owner[a.slot]==actor;}
int sx_allocate(struct sx_ledger*l,uint32_t id,size_t n,unsigned owner,unsigned allocator,struct sx_alias*out){
 struct sx_ledger next;unsigned slot=4,first_free=4;int rc;
 if(!l||!out||owner>1||allocator>1||!id||!n)return B4_INVALID;
 for(unsigned i=0;i<4;i++){if(l->ledger.slots[i].live&&l->ledger.slots[i].identity==id)return B4_STATE;if(!l->ledger.slots[i].live){if(first_free==4)first_free=i;if(slot==4&&l->generation[i]<3)slot=i;}}
 if(slot==4)return B4_RANGE;
 /* The baseline chooses first_free. After its complete valid-ledger operation,
  * move the newly modeled record to the selected nonretired free slot. This
  * permutation preserves the actual baseline live/peak accounting invariant. */
 next=*l;rc=b4_memory(&next.ledger,B4_ALLOC,id,n,1);if(rc!=B4_OK)return rc;
 if(slot!=first_free){struct b4_allocation tmp=next.ledger.slots[slot];next.ledger.slots[slot]=next.ledger.slots[first_free];next.ledger.slots[first_free]=tmp;}
 next.generation[slot]++;next.owner[slot]=owner;next.allocator[slot]=allocator;next.attributed[owner]+=n;*l=next;*out=(struct sx_alias){slot,next.generation[slot],id};return B4_OK;
}
int sx_resize(struct sx_ledger*l,struct sx_alias a,unsigned actor,size_t n,int succeeded,struct sx_alias*out){
 struct sx_ledger next;size_t old;int rc;
 if(!l||!out||!n||(succeeded!=0&&succeeded!=1))return B4_INVALID;
 if(!valid(l,a,actor))return B4_STATE;
 if(succeeded&&l->generation[a.slot]>=3)return B4_RANGE;
 next=*l;old=next.ledger.slots[a.slot].bytes;rc=b4_memory(&next.ledger,B4_RESIZE,a.identity,n,succeeded);if(rc!=B4_OK)return rc;
 if(succeeded){next.generation[a.slot]++;next.attributed[actor]=next.attributed[actor]-old+n;}
 *l=next;*out=(struct sx_alias){a.slot,next.generation[a.slot],a.identity};return B4_OK;
}
int sx_transfer(struct sx_ledger*l,struct sx_alias a,unsigned old,unsigned owner){
 size_t n;if(owner>1||!valid(l,a,old))return B4_STATE;n=l->ledger.slots[a.slot].bytes;
 if(old!=owner){l->attributed[old]-=n;l->attributed[owner]+=n;l->owner[a.slot]=owner;}
 return B4_OK;
}
int sx_release(struct sx_ledger*l,struct sx_alias a,unsigned actor,unsigned allocator){
 struct sx_ledger next;size_t n;int rc;if(!valid(l,a,actor)||allocator!=l->allocator[a.slot])return B4_STATE;
 next=*l;n=next.ledger.slots[a.slot].bytes;rc=b4_memory(&next.ledger,B4_FREE,a.identity,0,1);if(rc!=B4_OK)return rc;
 next.attributed[actor]-=n;*l=next;return B4_OK;
}
int sx_lookup(const struct sx_ledger*l,struct sx_alias a,unsigned actor,size_t*out){if(!out)return B4_INVALID;
#if SX_MUTANT == 3
 if(!l||a.slot>=4||!l->ledger.slots[a.slot].live||l->owner[a.slot]!=actor)return B4_STATE;
#else
 if(!valid(l,a,actor))return B4_STATE;
#endif
 *out=l->ledger.slots[a.slot].bytes;return B4_OK;}
int sx_online(struct sx_hotplug*w,unsigned cpu){unsigned bit;if(!w||cpu>=8)return S_INVALID;bit=1u<<cpu;
 if(w->request.active||(w->online&bit))return S_CONFLICT;
 if(w->incarnation[cpu]>=3)return S_RANGE;
 w->incarnation[cpu]++;w->online|=bit;w->draining&=~bit;w->quiescent&=~bit;return S_OK;
}
int sx_begin(struct sx_hotplug*w,unsigned targets){int rc;if(!w||targets>255)return S_INVALID;
 if(targets&~w->online||targets&w->draining)return S_CONFLICT;
 rc=s_shoot_begin(&w->request,targets);if(rc!=S_OK)return rc;
 for(unsigned i=0;i<8;i++)w->pending_incarnation[i]=w->incarnation[i];
 return S_OK;
}
int sx_offline_request(struct sx_hotplug*w,unsigned cpu,uint32_t incarnation){unsigned bit;if(!w||cpu>=8)return S_INVALID;bit=1u<<cpu;
 if(!(w->online&bit)||w->incarnation[cpu]!=incarnation)return S_CONFLICT;
 w->draining|=bit;
#if SX_MUTANT == 4
 w->request.pending&=~bit;w->online&=~bit;
#endif
 return S_OK;
}
int sx_offline_quiescent(struct sx_hotplug*w,unsigned cpu,uint32_t incarnation){unsigned bit;int rc;if(!w||cpu>=8)return S_INVALID;bit=1u<<cpu;
 if(!(w->draining&bit)||!(w->online&bit)||w->incarnation[cpu]!=incarnation)return S_CONFLICT;
 if(w->request.active&&(w->request.pending&bit)){if(w->pending_incarnation[cpu]!=incarnation)return S_CONFLICT;rc=s_shoot_ack(&w->request,cpu,w->request.generation);if(rc!=S_OK)return rc;}
 w->quiescent|=bit;w->online&=~bit;w->draining&=~bit;return S_OK;
}
int sx_ack(struct sx_hotplug*w,unsigned cpu,uint32_t incarnation,uint32_t generation){if(!w||cpu>=8)return S_INVALID;if(!(w->online&(1u<<cpu))||w->pending_incarnation[cpu]!=incarnation)return S_CONFLICT;return s_shoot_ack(&w->request,cpu,generation);}
int sx_reclaim(struct sx_hotplug*w){if(!w)return S_INVALID;return s_shoot_reclaim(&w->request);}
