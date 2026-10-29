#include "pool.h"
#ifndef SP_MUTANT
#define SP_MUTANT 0
#endif
static int sp_lock(struct sp_pool *p,struct sl_guard *g){*g=(struct sl_guard){0};return sl_enter(&p->lock,NULL,g,NULL)==SL_OK;}
static int sp_id(const struct sp_pool *p,struct sp_id id){return id.world==p->identity&&id.slot<SP_JOBS&&id.generation&&p->jobs[id.slot].generation==id.generation&&p->jobs[id.slot].state!=SP_FREE;}
static void sp_remove(struct sp_queue *q,unsigned index){for(unsigned i=index;i+1<q->count;i++)q->entries[i]=q->entries[i+1];q->count--;}
static void sp_unqueue(struct sp_pool *p,unsigned slot){struct sp_queue *q=&p->queues[p->jobs[slot].queue];for(unsigned i=0;i<q->count;i++)if(q->entries[i]==slot){sp_remove(q,i);return;}}
int sp_init(struct sp_pool *p,uint64_t world,unsigned workers,int deques){
 if(!p||!world||workers>4||(deques!=0&&deques!=1))return SP_INVALID;
 p->identity=world;p->accepted=p->completed=p->cancelled=0;p->workers=workers;p->deques=(unsigned)deques;p->closed=0;(void)sl_init(&p->lock,SL_TAS,1,0);
 for(unsigned i=0;i<SP_JOBS;i++)p->jobs[i]=(struct sp_job){0};
 for(unsigned i=0;i<4;i++)p->queues[i]=(struct sp_queue){0};
 return SP_OK;
}
int sp_submit(struct sp_pool *p,unsigned home,uint64_t payload,uint64_t cost,struct sp_id *out){
 if(!p||!out)return SP_INVALID;
 if(!p->workers)return SP_NO_WORKERS;
 if(home>=p->workers)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;int result=SP_OK;unsigned queue=p->deques?home:0;struct sp_queue *q=&p->queues[queue];unsigned slot=0;
 if(p->closed&&SP_MUTANT!=1)result=SP_CLOSED;
 else if(q->count==SP_QUEUE_CAPACITY||p->accepted==UINT64_MAX)result=SP_FULL;
 else{while(slot<SP_JOBS&&(p->jobs[slot].state!=SP_FREE||p->jobs[slot].generation==UINT64_MAX))slot++;if(slot==SP_JOBS)result=SP_FULL;}
 if(result==SP_OK){struct sp_job *j=&p->jobs[slot];uint64_t generation=j->generation+1;if(SP_MUTANT==6&&j->generation)generation=j->generation;*j=(struct sp_job){generation,payload,cost,0,SP_QUEUED,4,queue,0};q->entries[q->count++]=slot;p->accepted++;*out=(struct sp_id){slot,p->identity,generation};}
 (void)sl_leave(&g,NULL);return result;
}
int sp_claim(struct sp_pool *p,unsigned worker,struct sp_claim *out){
 if(!p||!out||worker>=p->workers)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;unsigned selected=p->deques?worker:0;struct sp_queue *q=&p->queues[selected];int local=1;
 if(p->deques&&!q->count){local=0;for(unsigned step=1;step<p->workers;step++){selected=(worker+step)%p->workers;q=&p->queues[selected];if(q->count)break;}}
 int result=SP_EMPTY;
 if(q->count){unsigned index=p->deques&&local?q->count-1:0;if(SP_MUTANT==5&&!local)index=q->count-1;unsigned slot=q->entries[index];sp_remove(q,index);struct sp_job *j=&p->jobs[slot];j->state=SP_MUTANT==2?SP_COMPLETED:SP_RUNNING;j->owner=worker;*out=(struct sp_claim){{slot,p->identity,j->generation},worker,j->payload,j->cost};result=SP_OK;}
 (void)sl_leave(&g,NULL);return result;
}
int sp_complete(struct sp_pool *p,struct sp_claim claim,uint64_t value){
 if(!p||claim.worker>=p->workers)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;int result=SP_STALE;
 if(sp_id(p,claim.id)){struct sp_job *j=&p->jobs[claim.id.slot];if(j->owner==claim.worker&&(j->state==SP_RUNNING||(SP_MUTANT==3&&j->state==SP_COMPLETED))){if(j->cancel){j->state=SP_CANCELLED;p->cancelled++;}else{j->state=SP_COMPLETED;j->result=value;p->completed++;}result=SP_OK;}}
 (void)sl_leave(&g,NULL);return result;
}
int sp_cancel(struct sp_pool *p,struct sp_id id){
 if(!p)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;int result=SP_STALE;
 if(sp_id(p,id)){struct sp_job *j=&p->jobs[id.slot];if(j->state==SP_QUEUED){sp_unqueue(p,id.slot);j->state=SP_CANCELLED;p->cancelled++;result=SP_OK;}else if(j->state==SP_RUNNING){j->cancel=1;if(SP_MUTANT==4){j->state=SP_CANCELLED;p->cancelled++;}result=SP_REQUESTED;}else result=SP_BUSY;}
 (void)sl_leave(&g,NULL);return result;
}
int sp_result(struct sp_pool *p,struct sp_id id,unsigned *state,uint64_t *out){
 if(!p||!state||!out)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;int result=SP_STALE;
 if(sp_id(p,id)){struct sp_job *j=&p->jobs[id.slot];if(j->state==SP_COMPLETED||j->state==SP_CANCELLED){*state=j->state;*out=j->result;result=SP_OK;}else result=SP_BUSY;}
 (void)sl_leave(&g,NULL);return result;
}
int sp_collect(struct sp_pool *p,struct sp_id id){
 if(!p)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;int result=SP_STALE;
 if(sp_id(p,id)){struct sp_job *j=&p->jobs[id.slot];if(j->state==SP_COMPLETED||j->state==SP_CANCELLED){j->state=SP_FREE;result=SP_OK;}else result=SP_BUSY;}
 (void)sl_leave(&g,NULL);return result;
}
int sp_close(struct sp_pool *p,int cancel){
 if(!p||(cancel!=0&&cancel!=1))return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;p->closed=1;
 if(cancel)for(unsigned i=0;i<SP_JOBS;i++){struct sp_job *j=&p->jobs[i];if(j->state==SP_QUEUED){sp_unqueue(p,i);j->state=SP_CANCELLED;p->cancelled++;}else if(j->state==SP_RUNNING)j->cancel=1;}
 (void)sl_leave(&g,NULL);return SP_OK;
}
int sp_snapshot(struct sp_pool *p,struct sp_snapshot *out){
 if(!p||!out)return SP_INVALID;
 struct sl_guard g;if(!sp_lock(p,&g))return SP_BUSY;struct sp_snapshot s={0};s.accepted=p->accepted;s.completed=p->completed;s.cancelled=p->cancelled;
 for(unsigned i=0;i<SP_JOBS;i++){unsigned state=p->jobs[i].state;s.queued+=state==SP_QUEUED;s.running+=state==SP_RUNNING;s.terminal+=state==SP_COMPLETED||state==SP_CANCELLED;}for(unsigned i=0;i<4;i++)s.queue_size[i]=p->queues[i].count;*out=s;(void)sl_leave(&g,NULL);return SP_OK;
}
int sp_launch_init(struct sp_launch *p,struct sp_launch_port port){
 if(!p||!port.allocate||!port.create||!port.join||!port.release)return SP_INVALID;
 p->port=port;atomic_init(&p->gate,0);p->count=p->allocated=p->started=p->phase=0;for(unsigned i=0;i<4;i++){p->stacks[i]=NULL;p->handles[i]=0;}return SP_OK;
}
static int sp_reap(struct sp_launch *p){
 for(unsigned i=0;i<p->count;i++)if(p->allocated&(1u<<i)){
  if(SP_MUTANT==7)p->port.release(p->port.context,i,p->stacks[i]);
  if(p->started&(1u<<i)){if(p->port.join(p->port.context,p->handles[i])&&SP_MUTANT!=8)continue;p->started&=~(1u<<i);}
  if(SP_MUTANT!=7)p->port.release(p->port.context,i,p->stacks[i]);
  p->allocated&=~(1u<<i);p->stacks[i]=NULL;
 }
 p->phase=p->allocated?2:3;return p->allocated?SP_DEGRADED:SP_OK;
}
int sp_launch(struct sp_launch *p,unsigned workers){
 if(!p||workers>4)return SP_INVALID;
 if(p->phase!=0&&p->phase!=3)return SP_BUSY;
 p->count=workers;atomic_store_explicit(&p->gate,0,memory_order_release);p->phase=4;
 for(unsigned i=0;i<workers;i++){
  void *stack=p->port.allocate(p->port.context,i);if(!stack)goto fail;p->stacks[i]=stack;p->allocated|=1u<<i;
  uintptr_t handle=0;if(p->port.create(p->port.context,i,stack,&handle))goto fail;p->handles[i]=handle;p->started|=1u<<i;
 }
 p->phase=1;atomic_store_explicit(&p->gate,1,memory_order_release);return SP_OK;
fail:
 if(SP_MUTANT!=9)atomic_store_explicit(&p->gate,2,memory_order_release);
 return sp_reap(p)==SP_OK?SP_FAILED:SP_DEGRADED;
}
int sp_shutdown(struct sp_launch *p){
 if(!p)return SP_INVALID;
 if(p->phase==4)return SP_BUSY;
 atomic_store_explicit(&p->gate,2,memory_order_release);return sp_reap(p);
}
