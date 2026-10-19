#include "active_import.h"
#ifndef PI_MUTANT
#define PI_MUTANT 0
#endif
int pa_active_init(PATaskActive*a,PFLock lock){if(!a||!lock.enter||!lock.leave)return PT_INVALID;*a=(PATaskActive){.lock=lock,.machine={PT_OFFLINE,0,0}};cmx_init(&a->kernel,CMX_FIXED);cmx_queue_init(&a->queue,4);for(unsigned i=0;i<4;i++)if(cmx_task_add(&a->kernel,i+1,100,(uintptr_t)a->stacks[i],sizeof a->stacks[i])!=(int)i)return PT_INVALID;return PT_OK;}
int pa_active_post(PATaskActive*a,unsigned producer,unsigned event){
 if(!a||producer>1||event>PT_PING)return PT_INVALID;a->lock.enter(a->lock.context);int r=PT_RANGE;
 if(a->queue.count+a->held<4||PI_MUTANT==2){enum cmx_result q=cmx_queue_send(&a->kernel,&a->queue,producer,event,cmx_at(a->kernel.now));r=q==CMX_OK?PT_OK:q==CMX_TIMEOUT?PT_RANGE:PT_STATE;}
 a->lock.leave(a->lock.context);return r;
}
static int take(PATaskActive*a,unsigned consumer){if(a->held)return PT_OK;enum cmx_result q=cmx_queue_recv(&a->kernel,&a->queue,consumer,cmx_at(a->kernel.now));if(q==CMX_TIMEOUT)return PT_PENDING;if(q!=CMX_OK)return PT_STATE;a->event=a->kernel.task[consumer].value;a->held=1;return PT_OK;}
int pa_active_step(PATaskActive*a,unsigned consumer,unsigned char*trace,size_t cap,size_t*used){
 if(!a||consumer<2||consumer>3||!used||(!trace&&cap))return PT_INVALID;a->lock.enter(a->lock.context);int r=take(a,consumer);
 if(!r){r=pt_dispatch(&a->machine,a->event,trace,cap,used);if(!r||PI_MUTANT==1)a->held=0;}
 a->lock.leave(a->lock.context);return r;
}
int pa_active_discard(PATaskActive*a,unsigned consumer,unsigned*out){if(!a||consumer<2||consumer>3||!out)return PT_INVALID;a->lock.enter(a->lock.context);int r=take(a,consumer);if(!r){*out=a->event;a->held=0;a->discarded++;}a->lock.leave(a->lock.context);return r;}
