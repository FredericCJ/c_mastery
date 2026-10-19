#include "cl_cmx.h"
#ifndef CC_MUTANT
#define CC_MUTANT 0
#endif
static int lease_equal(CLLease a,CLLease b){
  return a.object_generation==b.object_generation&&a.generation==b.generation&&a.slot==b.slot;
}
static int valid(CCHandle h){
  if(!h.owner||!h.owner->kernel||h.task>=CMX_TASKS)return 0;
  CCNode *n=&h.owner->nodes[h.task];cmx_task *t=&h.owner->kernel->task[h.task];
  return n->registered&&lease_equal(h.lease,n->lease)&&h.lease.object_generation==h.owner->life.generation&&
    h.generation==n->generation&&h.generation==t->generation&&t->state!=CMX_UNUSED&&t->state!=CMX_EXITED;
}
static int ticket_valid(CCTicket t){
  return valid(t.handle)&&(CC_MUTANT==1||t.serial==t.handle.owner->nodes[t.handle.task].serial)&&t.serial!=0;
}
static int available_task(CCHandle h){
  unsigned state=h.owner->kernel->task[h.task].state;
  return state==CMX_READY||state==CMX_RUNNING;
}
static unsigned lease_state(CCQueue *q,CCNode *n){return q->life.leases[n->lease.slot].state;}
static void active_lease(CCQueue *q,CCNode *n){
  enum cl_lease_state state=(enum cl_lease_state)lease_state(q,n);
  if(state!=CL_ACTIVE)(void)cl_transition(&q->life,n->lease,state,CL_ACTIVE);
}
static void terminal(CCQueue *q,unsigned task,int result){
  CCNode *n=&q->nodes[task];
  if(q->kernel->task[task].state==CMX_WAITING)cmx_latch_signal(q->kernel,&n->completion);
  n->result=result;n->phase=CC_TERMINAL;n->borrowed=0;
  /* A condition result has reacquired its actual mutex; its implicit lease
   * stays COND_RELOCK until the caller collects then unlocks it. */
  if(n->kind!=CC_CONDITION)active_lease(q,n);
}
static void settle(CCQueue *q){
  for(unsigned i=0;i<CMX_TASKS;i++){
    CCNode *n=&q->nodes[i];cmx_task *task=&q->kernel->task[i];
    if(n->phase!=CC_WAITING)continue;
    if(task->state==CMX_WAITING){
      if(n->kind==CC_CONDITION&&task->wait==CMX_W_MUTEX&&(task->flags&UINT32_C(0x80000000)))
        n->result=task->pending==CMX_OK?CL_OK:task->pending==CMX_TIMEOUT?CC_TIMEOUT:CC_CANCELLED;
      continue;
    }
    int result=n->kind==CC_CONDITION&&n->result!=CC_BLOCKED?n->result:
      task->result==CMX_OK?CL_OK:task->result==CMX_TIMEOUT?CC_TIMEOUT:CC_CANCELLED;
    terminal(q,i,result);
  }
}
static unsigned first(CCQueue *q,unsigned kind){
  unsigned chosen=CMX_TASKS;
  for(unsigned i=0;i<CMX_TASKS;i++)if(q->nodes[i].phase==CC_WAITING&&q->nodes[i].kind==kind&&
      (chosen==CMX_TASKS||q->nodes[i].order<q->nodes[chosen].order))chosen=i;
  return chosen;
}
static int deliver(CCQueue *q,CCNode *n){
  CLMessage value={0};int result=cl_recv(&q->bytes,value.bytes,n->capacity,&value.length,&value.producer,&value.sequence);
  if(result==CL_OK)n->received=value;
  return result;
}
int cc_progress(CCQueue *q){
  if(!q||!q->kernel||q->life.phase==CL_DESTROYED)return CC_INVALID;
  if(CC_MUTANT!=2)settle(q);
  /* Each successful loop terminalizes a distinct admitted node. */
  for(unsigned pass=0;pass<CMX_TASKS*2;pass++){
    unsigned receiver=first(q,CC_RECEIVE),sender=first(q,CC_SEND);int changed=0;
    if(receiver<CMX_TASKS&&(q->bytes.count||q->bytes.closed)){
      int result=deliver(q,&q->nodes[receiver]);terminal(q,receiver,result);changed=1;
    }
    if(sender<CMX_TASKS&&(q->bytes.count<2||q->bytes.closed)){
      CCNode *n=&q->nodes[sender];int result=cl_send(&q->bytes,n->producer,n->sequence,n->borrowed,n->length);
      terminal(q,sender,result);changed=1;
    }
    if(!changed)break;
  }
  return CL_OK;
}
int cc_init(CCQueue *q,cmx_kernel *kernel){
  if(!q||!kernel)return CC_INVALID;
  *q=(CCQueue){.kernel=kernel};cl_life_init(&q->life);cl_queue_init(&q->bytes,q->storage);return CL_OK;
}
int cc_register(CCQueue *q,unsigned task,CCHandle *out){
  if(!q||!q->kernel||!out||task>=CMX_TASKS||q->nodes[task].registered)return CC_INVALID;
  if(q->kernel->task[task].state!=CMX_READY&&q->kernel->task[task].state!=CMX_RUNNING)return CC_INVALID;
  CLLease lease;int result=cl_acquire(&q->life,&lease);if(result!=CL_OK)return result;
  CCNode *n=&q->nodes[task];n->registered=1;n->lease=lease;n->generation=q->kernel->task[task].generation;
  *out=(CCHandle){q,task,n->generation,lease};return CL_OK;
}
static int preflight(CCHandle h,CCTicket *out){
  if(!valid(h))return CL_STALE;
  (void)cc_progress(h.owner);
  CCNode *n=&h.owner->nodes[h.task];
  if(!out||n->phase!=CC_IDLE||!available_task(h))return CC_INVALID;
  if(n->serial==UINT32_MAX)return CC_RETIRED;
  return CL_OK;
}
static void admitted(CCHandle h,unsigned kind,int result,CCTicket *out){
  CCNode *n=&h.owner->nodes[h.task];n->serial++;n->kind=kind;n->result=result;
  n->phase=result==CC_BLOCKED?CC_WAITING:CC_TERMINAL;*out=(CCTicket){h,n->serial};
}
static int enroll(CCHandle h,unsigned kind,cmx_deadline deadline){
  CCQueue *q=h.owner;CCNode *n=&q->nodes[h.task];
  enum cl_lease_state state=kind==CC_SEND?CL_SEND_WAIT:CL_RECV_WAIT;
  int result=cl_transition(&q->life,n->lease,CL_ACTIVE,state);if(result!=CL_OK)return result;
  n->completion.signaled=0;
  result=cmx_latch_wait(q->kernel,&n->completion,h.task,deadline);
  if(result!=CMX_BLOCKED){(void)cl_transition(&q->life,n->lease,state,CL_ACTIVE);return result==CMX_TIMEOUT?CC_TIMEOUT:result==CMX_FULL?CC_RETIRED:CC_INVALID;}
  n->order=q->kernel->task[h.task].wait_sequence;return CC_BLOCKED;
}
int cc_send(CCHandle h,uint32_t producer,uint32_t sequence,const unsigned char *bytes,size_t length,cmx_deadline deadline,CCTicket *out){
  int result=preflight(h,out);if(result!=CL_OK)return result;
  if(length>4||(length&&!bytes))return CL_RANGE;
  CCQueue *q=h.owner;CCNode *n=&q->nodes[h.task];
  result=cl_send(&q->bytes,producer,sequence,bytes,length);
  if(result==CL_FULL)result=enroll(h,CC_SEND,deadline);
  if(result!=CL_OK&&result!=CC_BLOCKED)return result;
  n->producer=producer;n->sequence=sequence;n->length=length;n->borrowed=result==CC_BLOCKED?bytes:0;
  admitted(h,CC_SEND,result,out);(void)cc_progress(q);return result;
}
int cc_receive(CCHandle h,size_t capacity,cmx_deadline deadline,CCTicket *out){
  int result=preflight(h,out);if(result!=CL_OK)return result;
  CCQueue *q=h.owner;CCNode *n=&q->nodes[h.task];n->capacity=capacity;
  result=deliver(q,n);
  if(result==CL_EMPTY)result=enroll(h,CC_RECEIVE,deadline);
  if(result!=CL_OK&&result!=CC_BLOCKED)return result;
  admitted(h,CC_RECEIVE,result,out);(void)cc_progress(q);return result;
}
int cc_result(CCTicket ticket,CLMessage *out){
  if(!ticket_valid(ticket))return CL_STALE;
  CCQueue *q=ticket.handle.owner;(void)cc_progress(q);CCNode *n=&q->nodes[ticket.handle.task];
  if(n->phase==CC_WAITING)return CC_BLOCKED;
  if(n->phase!=CC_TERMINAL)return CL_STALE;
  if(n->kind==CC_RECEIVE&&n->result==CL_OK){if(!out)return CL_RANGE;*out=n->received;}
  int result=n->result;n->phase=n->kind==CC_CONDITION?CC_REACQUIRED:CC_IDLE;return result;
}
int cc_cancel(CCTicket ticket){
  if(!ticket_valid(ticket))return CL_STALE;
  CCQueue *q=ticket.handle.owner;(void)cc_progress(q);CCNode *n=&q->nodes[ticket.handle.task];
  if(n->phase==CC_TERMINAL)return n->result;
  if(n->phase!=CC_WAITING)return CL_STALE;
  if(n->kind==CC_CONDITION&&n->result!=CC_BLOCKED)return CC_BLOCKED;
  int result=cmx_cancel(q->kernel,ticket.handle.task);if(result!=CMX_OK)return CC_INVALID;
  (void)cc_progress(q);return n->phase==CC_WAITING?CC_BLOCKED:n->result;
}
int cc_close(CCQueue *q,unsigned abort_queued){
  if(!q||!q->kernel||abort_queued>1||q->life.phase==CL_DESTROYED)return CC_INVALID;
  /* Cache already selected timeout/cancel before touching borrowed inputs. */
  settle(q);if(abort_queued)(void)cl_abort(&q->bytes);else if(CC_MUTANT!=4)cl_close(&q->bytes);cl_life_close(&q->life);
  for(unsigned i=0;i<CMX_TASKS;i++)if(q->nodes[i].phase==CC_WAITING&&q->nodes[i].kind==CC_CONDITION&&q->nodes[i].result==CC_BLOCKED)(void)cmx_cancel(q->kernel,i);
  return cc_progress(q);
}
int cc_release(CCHandle *h){
  if(!h||!valid(*h))return CL_STALE;
  CCQueue *q=h->owner;(void)cc_progress(q);CCNode *n=&q->nodes[h->task];
  if(CC_MUTANT==3&&n->kind==CC_CONDITION&&n->phase==CC_WAITING){active_lease(q,n);n->phase=CC_IDLE;}
  if(q->life.phase==CL_OPEN||n->phase!=CC_IDLE)return CL_BUSY;
  int result=cl_drop(&q->life,n->lease);if(result==CL_OK){n->registered=0;*h=(CCHandle){0};}return result;
}
int cc_destroy(CCQueue *q){
  if(!q||!q->kernel||q->life.phase==CL_OPEN||q->life.phase==CL_DESTROYED)return CL_BUSY;
  (void)cc_progress(q);
  if(q->bytes.count)return CL_BUSY;
  for(unsigned i=0;i<CMX_TASKS;i++)if(q->nodes[i].phase!=CC_IDLE)return CL_BUSY;
  if(q->life.phase==CL_CLOSING)(void)cl_life_drained(&q->life);
  return cl_destroy(&q->life);
}
int cc_restart(CCQueue *q){
  if(!q||!q->kernel)return CC_INVALID;
  int result=cl_restart(&q->life);if(result!=CL_OK)return result;
  cl_queue_init(&q->bytes,q->storage);
  for(unsigned i=0;i<CMX_TASKS;i++)q->nodes[i]=(CCNode){0};
  return CL_OK;
}
int cc_condition(CCHandle h,cmx_cond *condition,cmx_mutex *mutex,cmx_deadline deadline,CCTicket *out){
  int result=preflight(h,out);if(result!=CL_OK)return result;
  if(!condition||!mutex)return CC_INVALID;
  CCQueue *q=h.owner;CCNode *n=&q->nodes[h.task];
  result=cl_transition(&q->life,n->lease,CL_ACTIVE,CL_COND_RELOCK);if(result!=CL_OK)return result;
  result=cmx_cond_wait(q->kernel,condition,mutex,h.task,deadline);
  if(result!=CMX_BLOCKED){(void)cl_transition(&q->life,n->lease,CL_COND_RELOCK,CL_ACTIVE);return result==CMX_TIMEOUT?CC_TIMEOUT:result==CMX_FULL?CC_RETIRED:result==CMX_CLOSED?CL_CLOSED:CC_INVALID;}
  n->reacquire=mutex;admitted(h,CC_CONDITION,CC_BLOCKED,out);return CC_BLOCKED;
}
int cc_condition_unlock(CCHandle h){
  if(!valid(h))return CL_STALE;
  CCQueue *q=h.owner;CCNode *n=&q->nodes[h.task];
  if(n->phase!=CC_REACQUIRED||!n->reacquire)return CL_BUSY;
  if(cmx_mutex_unlock(q->kernel,n->reacquire,h.task)!=CMX_OK)return CC_INVALID;
  n->reacquire=0;n->phase=CC_IDLE;active_lease(q,n);return CL_OK;
}
static int port_valid(CCPort p){
  if(!p.owner||p.lease.slot>=8||p.lease.object_generation!=p.owner->life.generation)return 0;
  CLLeaseSlot *s=&p.owner->life.leases[p.lease.slot];return s->generation==p.lease.generation&&s->state==CL_TIMER;
}
int cc_port_acquire(CCQueue *q,CCPort *out){
  if(!q||!q->kernel||!out)return CC_INVALID;
  CLLease lease;int result=cl_acquire(&q->life,&lease);if(result!=CL_OK)return result;
  result=cl_transition(&q->life,lease,CL_ACTIVE,CL_TIMER);
  if(result!=CL_OK){(void)cl_drop(&q->life,lease);return result;}
  *out=(CCPort){q,lease};return CL_OK;
}
int cc_port_send(CCPort p,uint32_t producer,uint32_t sequence,const unsigned char *bytes,size_t length){
  if(!port_valid(p))return CL_STALE;
  (void)cc_progress(p.owner);int result=cl_send(&p.owner->bytes,producer,sequence,bytes,length);(void)cc_progress(p.owner);return result;
}
int cc_port_release_after_quiescence(CCPort *p){
  if(!p||!port_valid(*p))return CL_STALE;
  int result=cl_transition(&p->owner->life,p->lease,CL_TIMER,CL_ACTIVE);if(result!=CL_OK)return result;
  result=cl_drop(&p->owner->life,p->lease);if(result==CL_OK)*p=(CCPort){0};return result;
}
