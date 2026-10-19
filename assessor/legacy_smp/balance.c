#include "balance.h"
#include <stddef.h>
#ifndef SB_MUTANT
#define SB_MUTANT 0
#endif
static int sb_valid(const SbWorld *w,unsigned actor){return w&&w->wake.queues.cpus&&w->wake.queues.cpus<=SQ_CPUS&&actor<w->wake.queues.cpus;}
static int sb_mask(const SbWorld *w,unsigned mask){return mask&&!(mask&~((1u<<w->wake.queues.cpus)-1));}
static int sb_enter(SbWorld *w,SlGuard *guard,SlStats *s){*guard=(SlGuard){0};return sl_enter(&w->policy,NULL,guard,s)==SL_OK;}
static unsigned sb_state(const SqTask *t){return atomic_load_explicit(&t->state,memory_order_acquire);}
static unsigned sb_home(const SqTask *t){return atomic_load_explicit(&t->home,memory_order_acquire);}
static int sb_match(const SbWorld *w,SqHandle id){return id.slot<SQ_TASKS&&id.generation&&w->wake.queues.task[id.slot].generation==id.generation;}
int sb_init(SbWorld *w,unsigned cpus,unsigned capacity,SwPort port){
 if(!w)return SQ_INVALID;
 int r=sw_init(&w->wake,cpus,capacity,port);if(r!=SQ_OK)return r;
 (void)sl_init(&w->policy,SL_TAS,1,0);
 for(unsigned i=0;i<SQ_TASKS;i++){w->mask[i]=w->pending[i]=w->timer_live[i]=0;w->timer_serial[i]=w->migrations[i]=0;}return SQ_OK;
}
int sb_create(SbWorld *w,unsigned actor,unsigned home,unsigned slot,unsigned mask,SqHandle *id,SqEvent *e){
 if(!sb_valid(w,actor)||home>=w->wake.queues.cpus||slot>=SQ_TASKS||!sb_mask(w,mask)||!(mask&(1u<<home))||!id||!e)return SQ_INVALID;
 if(w->timer_live[slot])return SW_BUSY;
 int r=sw_create(&w->wake,actor,home,slot,id,e);if(r==SQ_OK){w->mask[slot]=mask;w->pending[slot]=0;w->migrations[slot]=0;}return r;
}
int sb_affinity(SbWorld *w,unsigned actor,SqHandle id,unsigned mask,SlStats *s){
 if(!sb_valid(w,actor)||id.slot>=SQ_TASKS||!sb_mask(w,mask))return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 SqTask *t=&w->wake.queues.task[id.slot];int r=SQ_OK;unsigned state=sb_state(t);
 if(!sb_match(w,id))r=SQ_STALE;
 else if(state==SQ_TRANSFER)r=SW_MOVING;
 else if(state==SQ_EMPTY||state==SQ_EXITED)r=SQ_STATE;
 else if(state==SQ_RUNNING&&!(mask&(1u<<sb_home(t)))&&SB_MUTANT!=1){w->pending[id.slot]=mask;r=SB_DEFERRED;}
 else {w->mask[id.slot]=mask;w->pending[id.slot]=0;}
 (void)sl_leave(&g,s);return r;
}
int sb_dispatch(SbWorld *w,unsigned cpu,SqHandle *id,SqEvent *e,SlStats *s){
 if(!sb_valid(w,cpu)||!id||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 SqQueue *q=&w->wake.queues.queue[cpu];int r;
 if(q->count&&!(w->mask[q->slots[0]]&(1u<<cpu)))r=SB_AFFINITY;
 else r=sw_dispatch(&w->wake,cpu,id,e,s);
 (void)sl_leave(&g,s);return r;
}
int sb_finish(SbWorld *w,unsigned cpu,SqHandle id,unsigned next,SqEvent *e,SlStats *s){
 if(!sb_valid(w,cpu)||id.slot>=SQ_TASKS||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 int r=sw_finish(&w->wake,cpu,id,next,e,s);
 if(r==SQ_OK&&w->pending[id.slot]){w->mask[id.slot]=w->pending[id.slot];w->pending[id.slot]=0;}
 (void)sl_leave(&g,s);return r;
}
static int sb_wake_locked(SbWorld *w,unsigned actor,SqHandle id,SqEvent *e,SlStats *s){
 if(!sb_match(w,id))return SQ_STALE;
 return sw_wake(&w->wake,actor,sb_home(&w->wake.queues.task[id.slot]),id,e,s);
}
int sb_wake(SbWorld *w,unsigned actor,SqHandle id,SqEvent *e,SlStats *s){
 if(!sb_valid(w,actor)||id.slot>=SQ_TASKS||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 int r=sb_wake_locked(w,actor,id,e,s);(void)sl_leave(&g,s);return r;
}
static int sb_begin_locked(SbWorld *w,unsigned actor,unsigned dst,SqHandle id,unsigned fail,SqTransfer *ticket,SqEvent *e,SlStats *s){
 if(!sb_match(w,id))return SQ_STALE;
 unsigned src=sb_home(&w->wake.queues.task[id.slot]);
 if(!(w->mask[id.slot]&(1u<<dst)))return SB_AFFINITY;
 if(w->migrations[id.slot]==UINT64_MAX)return SQ_FULL;
 if(fail==1)return SB_INJECTED;
 SqTransfer staged;SqEvent event;int r=sw_transfer_begin(&w->wake,actor,src,dst,id,&staged,&event,s);
 if(r!=SQ_OK)return r;
 if(fail==2){if(SB_MUTANT==2)return SB_INJECTED;int undone=sw_transfer_end(&w->wake,actor,staged,0,&event,s);return undone==SQ_OK||undone==SW_SIGNAL?SB_INJECTED:undone;}
 *ticket=staged;*e=event;return SQ_OK;
}
int sb_move_begin(SbWorld *w,unsigned actor,unsigned dst,SqHandle id,unsigned fail,SqTransfer *ticket,SqEvent *e,SlStats *s){
 if(!sb_valid(w,actor)||dst>=w->wake.queues.cpus||id.slot>=SQ_TASKS||fail>2||!ticket||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 int r=sb_begin_locked(w,actor,dst,id,fail,ticket,e,s);(void)sl_leave(&g,s);return r;
}
static int sb_end_locked(SbWorld *w,unsigned actor,SqTransfer ticket,int commit,SqEvent *e,SlStats *s){
 int r=sw_transfer_end(&w->wake,actor,ticket,commit,e,s);if((r==SQ_OK||r==SW_SIGNAL)&&commit)w->migrations[ticket.task.slot]++;return r;
}
int sb_move_end(SbWorld *w,unsigned actor,SqTransfer ticket,int commit,SqEvent *e,SlStats *s){
 if(!sb_valid(w,actor)||ticket.task.slot>=SQ_TASKS||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 int r=sb_end_locked(w,actor,ticket,commit,e,s);(void)sl_leave(&g,s);return r;
}
int sb_balance(SbWorld *w,unsigned actor,SqEvent *begin,SqEvent *end,SlStats *s){
 if(!sb_valid(w,actor)||!begin||!end)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;
 unsigned chosen=SQ_NONE,destination=0;SqWorld *q=&w->wake.queues;
 for(unsigned forced=1;forced<3&&chosen==SQ_NONE;forced++)for(unsigned slot=0;slot<SQ_TASKS&&chosen==SQ_NONE;slot++){
  SqTask *t=&q->task[slot];if(sb_state(t)!=SQ_READY)continue;
  unsigned src=sb_home(t),bad=!(w->mask[slot]&(1u<<src)),best=q->cpus,load=q->capacity+1;
  if((forced==1)!=bad)continue;
  for(unsigned cpu=0;cpu<q->cpus;cpu++){unsigned used=q->queue[cpu].count+q->queue[cpu].reserved;if(cpu!=src&&(SB_MUTANT==4||(w->mask[slot]&(1u<<cpu)))&&used<q->capacity&&used<load){best=cpu;load=used;}}
  if(best<q->cpus&&(bad||q->queue[src].count+q->queue[src].reserved>load+1)){chosen=slot;destination=best;}
 }
 int r=SQ_IDLE;if(chosen!=SQ_NONE){SqTransfer t;SqEvent first,last;SqHandle id={chosen,q->task[chosen].generation};unsigned saved=w->mask[chosen];if(SB_MUTANT==4)w->mask[chosen]=(1u<<q->cpus)-1;r=sb_begin_locked(w,actor,destination,id,0,&t,&first,s);w->mask[chosen]=saved;if(r==SQ_OK){r=sb_end_locked(w,actor,t,1,&last,s);if(r==SQ_OK||r==SW_SIGNAL){*begin=first;*end=last;}}}
 (void)sl_leave(&g,s);return r;
}
int sb_timer_arm(SbWorld *w,unsigned actor,SqHandle id,SbTimer *token,SlStats *s){
 if(!sb_valid(w,actor)||id.slot>=SQ_TASKS||!token)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;int r=SQ_OK;
 if(!sb_match(w,id))r=SQ_STALE;
 else if(sb_state(&w->wake.queues.task[id.slot])==SQ_EXITED)r=SQ_STATE;
 else if(w->timer_live[id.slot])r=SW_BUSY;
 else if(w->timer_serial[id.slot]==UINT64_MAX)r=SQ_FULL;
 else {w->timer_live[id.slot]=1;w->timer_serial[id.slot]++;*token=(SbTimer){id,w->timer_serial[id.slot],sb_home(&w->wake.queues.task[id.slot])};}
 (void)sl_leave(&g,s);return r;
}
static int sb_timer_valid(const SbWorld *w,SbTimer token){return sb_match(w,token.task)&&token.serial&&w->timer_live[token.task.slot]&&w->timer_serial[token.task.slot]==token.serial;}
int sb_timer_fire(SbWorld *w,unsigned actor,SbTimer token,SqEvent *e,SlStats *s){
 if(!sb_valid(w,actor)||token.task.slot>=SQ_TASKS||!e)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;int r=SQ_STALE;
 if(sb_timer_valid(w,token)){
  r=SB_MUTANT==3?sw_wake(&w->wake,actor,token.old_home,token.task,e,s):sb_wake_locked(w,actor,token.task,e,s);
  if(r==SQ_OK||r==SQ_DUPLICATE||r==SW_SIGNAL)w->timer_live[token.task.slot]=0;
 }
 (void)sl_leave(&g,s);return r;
}
int sb_timer_cancel(SbWorld *w,unsigned actor,SbTimer token,SlStats *s){
 if(!sb_valid(w,actor)||token.task.slot>=SQ_TASKS)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;int r=SQ_STALE;
 if(sb_timer_valid(w,token)){w->timer_live[token.task.slot]=0;r=SQ_OK;}(void)sl_leave(&g,s);return r;
}
int sb_retry(SbWorld *w,unsigned actor,unsigned dst,SlStats *s){
 if(!sb_valid(w,actor)||dst>=w->wake.queues.cpus)return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;int r=sw_retry(&w->wake,actor,dst,s);(void)sl_leave(&g,s);return r;
}
int sb_prepare_idle(SbWorld *w,unsigned cpu,int masked,SlStats *s){
 if(!sb_valid(w,cpu))return SQ_INVALID;
 SlGuard g;if(!sb_enter(w,&g,s))return SQ_STATE;int r=sw_prepare_idle(&w->wake,cpu,masked,s);(void)sl_leave(&g,s);return r;
}
int sb_invariant(const SbWorld *w){
 if(!w||!sq_invariant(&w->wake.queues))return 0;
 for(unsigned i=0;i<SQ_TASKS;i++){const SqTask *t=&w->wake.queues.task[i];unsigned state=sb_state(t);if(state==SQ_EMPTY)continue;if(!sb_mask(w,w->mask[i])||(w->pending[i]&&(state!=SQ_RUNNING||!sb_mask(w,w->pending[i]))))return 0;if(state==SQ_RUNNING&&!(w->mask[i]&(1u<<sb_home(t))))return 0;if(state==SQ_TRANSFER&&!(w->mask[i]&(1u<<t->destination)))return 0;}
 return 1;
}
