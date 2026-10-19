#include "treiber.h"
#ifndef TS_MUTANT
#define TS_MUTANT 0
#endif
TsStatus ts_init(TsStack *s,unsigned n){
 if(!s||!n||n>TS_NODES)return TS_INVALID;
 s->capacity=n;atomic_init(&s->head,TS_NONE);
 int supported=atomic_is_lock_free(&s->head);
 for(unsigned p=0;p<TS_THREADS;p++){atomic_init(&s->hazard[p],TS_NONE);if(!atomic_is_lock_free(&s->hazard[p]))supported=0;}
 for(unsigned i=0;i<TS_NODES;i++){
  atomic_init(&s->node[i].state,0);s->node[i].next=TS_NONE;s->node[i].value=0;
  if(!atomic_is_lock_free(&s->node[i].state))supported=0;
 }
 return supported?TS_OK:TS_UNSUPPORTED;
}
TsStatus ts_push(TsStack *s,uint32_t value){
 if(!s)return TS_INVALID;
 unsigned n=TS_NONE;
 for(unsigned i=0;i<s->capacity;i++){
  unsigned old=atomic_load(&s->node[i].state);
  if((old&3u)==0&&atomic_compare_exchange_strong(&s->node[i].state,&old,old|1u)){n=i;break;}
 }
 if(n==TS_NONE)return TS_FULL;
 s->node[n].value=value;
 unsigned head=atomic_load(&s->head);
 do {s->node[n].next=TS_MUTANT==1?TS_NONE:head;}
 while(!atomic_compare_exchange_strong_explicit(&s->head,&head,n,
         TS_MUTANT==5?memory_order_relaxed:memory_order_seq_cst,
         TS_MUTANT==5?memory_order_relaxed:memory_order_seq_cst));
 return TS_OK;
}
TsStatus ts_begin(TsStack *s,unsigned p,TsPop *out){
 if(!s||!out||p>=TS_THREADS)return TS_INVALID;
 unsigned n;
 do {
  n=atomic_load(&s->head);
  atomic_store(&s->hazard[p],TS_MUTANT==3?TS_NONE:n);
 } while(TS_MUTANT!=6&&n!=atomic_load(&s->head));
 if(n==TS_NONE)return TS_EMPTY;
 /* Hazard remains live through the later CAS and final payload access. */
 out->owner=s;out->participant=p;out->node=n;
 out->next=s->node[n].next;out->value=s->node[n].value;
 return TS_OK;
}
TsStatus ts_cancel(TsPop *v){
 if(!v||!v->owner)return TS_INVALID;
 atomic_store(&v->owner->hazard[v->participant],TS_NONE);v->owner=0;return TS_OK;
}
TsStatus ts_commit(TsPop *v,uint32_t *out){
 if(!v||!v->owner||!out)return TS_INVALID;
 TsStack *s=v->owner;unsigned expected=v->node;
 if(!atomic_compare_exchange_strong(&s->head,&expected,v->next)){
  (void)ts_cancel(v);return TS_RETRY;
 }
 /* Exact node generation cannot change while this live hazard protects it. */
 atomic_fetch_add(&s->node[v->node].state,1u);
 uint32_t value=v->value;
 (void)ts_cancel(v);*out=TS_MUTANT==2?value^1u:value;return TS_OK;
}
TsStatus ts_pop(TsStack *s,unsigned p,uint32_t *out){
 if(!s||!out||p>=TS_THREADS)return TS_INVALID;
 for(;;){TsPop v;TsStatus r=ts_begin(s,p,&v);if(r!=TS_OK)return r;
  r=ts_commit(&v,out);if(r!=TS_RETRY)return r;}
}
TsStatus ts_scan(TsStack *s,unsigned n,TsRetired *out){
 if(!s||!out||n>=s->capacity)return TS_INVALID;
 unsigned state=atomic_load(&s->node[n].state);
 if((state&3u)!=2u||state/4u==TS_GEN_MAX)return TS_RETRY;
 for(unsigned p=0;p<TS_THREADS;p++)if(atomic_load(&s->hazard[p])==n)return TS_RETRY;
 out->owner=s;out->node=n;out->state=state;return TS_OK;
}
TsStatus ts_reclaim(TsRetired *r){
 if(!r||!r->owner)return TS_INVALID;
 TsStack *s=r->owner;unsigned expected=r->state,n=r->node;r->owner=0;
 if(TS_MUTANT==4)expected=atomic_load(&s->node[n].state);
 if((expected&3u)!=2u||expected/4u==TS_GEN_MAX)return TS_RETRY;
 return atomic_compare_exchange_strong(&s->node[n].state,&expected,(expected&~3u)+4u)?TS_OK:TS_RETRY;
}
unsigned ts_collect(TsStack *s){
 if(!s)return 0;
 unsigned count=0;
 for(unsigned i=0;i<s->capacity;i++){TsRetired r;if(ts_scan(s,i,&r)==TS_OK&&ts_reclaim(&r)==TS_OK)count++;}
 return count;
}
