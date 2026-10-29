#include "startup.h"
#ifndef SS_MUTANT
#define SS_MUTANT 0
#endif
static int ticket(struct ss_world *w,struct ss_ticket t,struct ss_slot **out){
 if(!w||t.slot>=SS_SLOTS)return SS_INVALID;
 struct ss_slot *s=&w->slot[t.slot];if(s->phase==SS_EMPTY||(SS_MUTANT!=1&&s->generation!=t.generation))return SS_STALE;
 *out=s;return SS_OK;
}
int ss_begin(struct ss_world *w,unsigned slot,unsigned index,unsigned apic,uint64_t page,uint64_t low,uint64_t high,struct ss_ticket *out){
 if(!w||!out||slot>=SS_SLOTS||w->parallel>1||index<1||index>3||apic>255||page<4096||page>=0x100000||(page&4095)||low>=high||(low&15)||(high&15)||high-low<4096)return SS_INVALID;
 if(low<page+4096&&page<high)return SS_INVALID;
 if(w->slot[slot].phase!=SS_EMPTY)return SS_BUSY;
 if(w->slot[slot].generation==UINT32_MAX)return SS_FULL;
 for(unsigned i=0;i<SS_SLOTS;++i){struct ss_slot *s=&w->slot[i];if(s->phase==SS_EMPTY)continue;
  if(!w->parallel||s->apic==apic||s->index==index||s->page==page||(SS_MUTANT!=3&&low<s->stack_high&&s->stack_low<high)||(low<s->page+4096&&s->page<high)||(page<s->stack_high&&s->stack_low<page+4096))return SS_BUSY;
 }
 struct ss_slot next={w->slot[slot].generation+1,SS_PENDING,index,apic,page,low,high};w->slot[slot]=next;*out=(struct ss_ticket){slot,next.generation};return SS_OK;
}
int ss_timeout(struct ss_world *w,struct ss_ticket t){struct ss_slot *s;int r=ticket(w,t,&s);if(r)return r;if(s->phase!=SS_PENDING)return SS_BUSY;s->phase=SS_MUTANT==2?SS_QUIESCENT:SS_TIMED_OUT;return SS_OK;}
int ss_ack(struct ss_world *w,struct ss_ticket t,struct ss_evidence e){
 struct ss_slot *s;int r=ticket(w,t,&s);if(r)return r;
 if(s->phase==SS_READY||s->phase==SS_LATE_READY||s->phase==SS_QUIESCENT)return SS_DUPLICATE;
 if(e.apic!=s->apic||e.verified!=SS_VERIFIED||e.entry_stack!=s->stack_high-8)return SS_INVALID;
 int late=s->phase==SS_TIMED_OUT;s->phase=late?SS_LATE_READY:SS_READY;return late?SS_LATE:SS_OK;
}
int ss_quiescent(struct ss_world *w,struct ss_ticket t,int proof){struct ss_slot *s;int r=ticket(w,t,&s);if(r)return r;if(proof!=1)return SS_NOT_QUIESCENT;if(s->phase!=SS_READY&&s->phase!=SS_LATE_READY)return SS_BUSY;s->phase=SS_QUIESCENT;return SS_OK;}
int ss_release(struct ss_world *w,struct ss_ticket t){struct ss_slot *s;int r=ticket(w,t,&s);if(r)return r;if(s->phase!=SS_QUIESCENT)return SS_NOT_QUIESCENT;uint32_t generation=s->generation;*s=(struct ss_slot){0};s->generation=generation;return SS_OK;}
