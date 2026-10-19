#include "wakeup.h"
#include <stddef.h>
#ifndef SW_MUTANT
#define SW_MUTANT 0
#endif
static int sw_valid(const SwWorld *w,unsigned a,unsigned b){return w&&w->queues.cpus&&w->queues.cpus<=SQ_CPUS&&a<w->queues.cpus&&b<w->queues.cpus;}
static int sw_enter(SwWorld *w,SlGuard *g,SlStats *s){*g=(SlGuard){0};return sl_enter(&w->control,NULL,g,s)==SL_OK;}
static void sw_publish(SwWorld *w,unsigned c){atomic_store_explicit(&w->work[c],w->queues.queue[c].count,memory_order_release);}
static int sw_ring(SwWorld *w,unsigned c,int remote){
 if(!w->queues.queue[c].count)return SQ_OK;
 unsigned old=w->bell[c];w->bell[c]=1;
 if(!old&&remote){atomic_thread_fence(memory_order_seq_cst);if(!w->port.ring(w->port.context,c))return SW_SIGNAL;}
 return SQ_OK;
}
int sw_init(SwWorld *w,unsigned n,unsigned capacity,SwPort port){
 if(!w||!port.ring||!port.ack)return SQ_INVALID;
 int r=sq_init(&w->queues,n,capacity,0);if(r!=SQ_OK)return r;
 (void)sl_init(&w->control,SL_TAS,1,0);w->port=port;
 for(unsigned c=0;c<SQ_CPUS;c++){w->bell[c]=0;atomic_init(&w->work[c],0);}return SQ_OK;
}
int sw_create(SwWorld *w,unsigned actor,unsigned home,unsigned slot,SqHandle *id,SqEvent *event){
 if(!sw_valid(w,actor,home))return SQ_INVALID;
 int r=sq_create(&w->queues,actor,home,slot,id,event);if(r==SQ_OK){sw_publish(w,home);w->bell[home]=1;}return r;
}
int sw_wake(SwWorld *w,unsigned actor,unsigned home,SqHandle id,SqEvent *event,SlStats *s){
 if(!sw_valid(w,actor,home)||id.slot>=SQ_TASKS||!event)return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 SqTask *t=&w->queues.task[id.slot];unsigned state=atomic_load_explicit(&t->state,memory_order_acquire);int r;
 if(!id.generation||t->generation!=id.generation||atomic_load_explicit(&t->home,memory_order_acquire)!=home)r=SQ_STALE;
 else if(state==SQ_TRANSFER)r=SW_MOVING;
 else if(state==SQ_RUNNING)r=SQ_DUPLICATE;
 else {
  if(SW_MUTANT==1&&state==SQ_BLOCKED)(void)w->port.ring(w->port.context,home);
  r=sq_wake(&w->queues,actor,home,id,event,s);
  if(r==SQ_OK){sw_publish(w,home);if(SW_MUTANT==1)w->bell[home]=1;else r=sw_ring(w,home,1);}
 }
 (void)sl_leave(&guard,s);return r;
}
int sw_dispatch(SwWorld *w,unsigned cpu,SqHandle *id,SqEvent *event,SlStats *s){
 if(!sw_valid(w,cpu,cpu))return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 int r=sq_dispatch(&w->queues,cpu,id,event,s);if(r==SQ_OK)sw_publish(w,cpu);(void)sl_leave(&guard,s);return r;
}
int sw_finish(SwWorld *w,unsigned cpu,SqHandle id,unsigned next,SqEvent *event,SlStats *s){
 if(!sw_valid(w,cpu,cpu))return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 int r=sq_finish(&w->queues,cpu,id,next,event,s);if(r==SQ_OK){sw_publish(w,cpu);(void)sw_ring(w,cpu,0);}(void)sl_leave(&guard,s);return r;
}
int sw_transfer_begin(SwWorld *w,unsigned actor,unsigned src,unsigned dst,SqHandle id,SqTransfer *ticket,SqEvent *event,SlStats *s){
 if(!sw_valid(w,actor,src)||dst>=w->queues.cpus)return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 int r=sq_transfer_begin(&w->queues,actor,src,dst,id,ticket,event,s);if(r==SQ_OK){sw_publish(w,src);sw_publish(w,dst);}(void)sl_leave(&guard,s);return r;
}
int sw_transfer_end(SwWorld *w,unsigned actor,SqTransfer ticket,int commit,SqEvent *event,SlStats *s){
 if(!sw_valid(w,actor,ticket.source)||ticket.destination>=w->queues.cpus)return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 int r=sq_transfer_end(&w->queues,actor,ticket,commit,event,s);
 if(r==SQ_OK){sw_publish(w,ticket.source);sw_publish(w,ticket.destination);r=sw_ring(w,commit?ticket.destination:ticket.source,1);}
 (void)sl_leave(&guard,s);return r;
}
int sw_retry(SwWorld *w,unsigned actor,unsigned dst,SlStats *s){
 if(!sw_valid(w,actor,dst))return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 int r=SQ_IDLE;if(w->queues.queue[dst].count){w->bell[dst]=1;atomic_thread_fence(memory_order_seq_cst);r=w->port.ring(w->port.context,dst)?SQ_OK:SW_SIGNAL;}(void)sl_leave(&guard,s);return r;
}
int sw_prepare_idle(SwWorld *w,unsigned cpu,int masked,SlStats *s){
 if(!sw_valid(w,cpu,cpu)||masked!=1)return SQ_INVALID;
 SlGuard guard;if(!sw_enter(w,&guard,s))return SQ_STATE;
 /* Deassert the level, then check durable work while publication is excluded.
  * A producer after unlock reasserts; a pending fixed IPI survives IF=0. */
 w->port.ack(w->port.context,cpu);w->bell[cpu]=0;int r;
 if(w->queues.queue[cpu].current!=SQ_NONE)r=SW_BUSY;
 else if(SW_MUTANT!=2&&w->queues.queue[cpu].count){r=sw_ring(w,cpu,1);if(r==SQ_OK)r=SW_BUSY;}
 else r=SQ_IDLE;
 (void)sl_leave(&guard,s);return r;
}
