#include "abi.h"
#include "request.c"
/* Two independent three-descriptor chains in the retained eight-entry queue. */
struct rqa_request {
  Rq control;unsigned owner,owner_generation,waiter,type;
  uint64_t buffer,deadline;struct {uint32_t type,reserved;uint64_t sector;} header;
  volatile unsigned char status;
};
static struct rqa_request requests[2];
static unsigned char rqa_data[2][4096] ALIGN(4096);
static unsigned async_completions,async_wait_blocks,rqa_hold,rqa_resetting;
static uint64_t rqa_ticket(unsigned i){return ((uint64_t)requests[i].control.generation<<1)|i;}
static unsigned rqa_lookup(uint64_t ticket){
  unsigned i=(unsigned)(ticket&1),pid=uthreads[current].process;
  return ticket==rqa_ticket(i)&&requests[i].control.phase!=RQ_FREE&&requests[i].control.phase!=RQ_RETIRED&&
    requests[i].owner==pid&&requests[i].owner_generation==tasks[pid].generation?i:2;
}
static void rqa_pending(unsigned pid){
  tasks[pid].io_pending=0;
  for(unsigned i=0;i<2;i++)if(requests[i].owner==pid&&rq_owned(&requests[i].control))tasks[pid].io_pending=1;
}
static void rqa_publish(unsigned i){
  struct rqa_request *r=&requests[i];async_pending--;async_completions++;rqa_pending(r->owner);
  if(r->waiter<THREADS&&uthreads[r->waiter].state==UT_IO){
    uthreads[r->waiter].frame->rax=(uint64_t)(int64_t)r->control.result;uthreads[r->waiter].state=UT_READY;
    if(rq_apply(&r->control,RQ_REAP,r->control.generation,0))panic("rqa waiter reap");
  }else if(!tasks[r->owner].alive){if(rq_apply(&r->control,RQ_REAP,r->control.generation,0))panic("rqa dead reap");}
}
static void async_poll(void){
  if(!async_pending||rqa_hold||rqa_resetting)return;
  barrier();
  /* Interrupts are hints; consume every used element, including coalescing. */
  while(used_idx!=vu.idx){
    if((uint16_t)(vu.idx-used_idx)>2)panic("rqa used count");
    unsigned id=vu.ring[used_idx%8].id;
    if(id!=0&&id!=3)panic("rqa used descriptor");
    unsigned i=id/3;struct rqa_request *r=&requests[i];
    if(!rq_owned(&r->control))panic("rqa duplicate completion");
    int result=r->status? -1:0;
#ifdef RQA_MUTANT_STATUS
    result=0;
#endif
    if(!result&&r->type==0&&tasks[r->owner].alive&&tasks[r->owner].generation==r->owner_generation){
      struct task *p=&tasks[r->owner];
      if(!user_ok(p,r->buffer,4096,1))result=-1;
      else for(unsigned j=0;j<4096;j++)*(unsigned char *)physical(p,r->buffer+j)=rqa_data[i][j];
    }
    if(rq_apply(&r->control,RQ_COMPLETE,r->control.generation,result))panic("rqa completion state");
    used_idx++;rqa_publish(i);
  }
  for(unsigned i=0;i<2;i++)if((requests[i].control.phase==RQ_INFLIGHT||requests[i].control.phase==RQ_CANCELLED)&&ticks>requests[i].deadline)
    (void)rq_apply(&requests[i].control,RQ_TIMEOUT,requests[i].control.generation,0);
}
static int64_t async_submit(uint64_t handle,uint64_t pointer){
  unsigned pid=uthreads[current].process;struct task *p=&tasks[pid];
  if(handle>UINT32_MAX||!cmk_handle_valid((uint32_t)handle,0,p->handle_generation,p->handle_open))return -1;
  if(!disk_ready||rqa_resetting||!user_ok(p,pointer,sizeof(struct cmk2_block_request),0))return -1;
  struct cmk2_block_request input;memcpy(&input,(void *)(uintptr_t)pointer,sizeof input);
  if(input.type!=0&&input.type!=1&&input.type!=4)return -1;
  if(input.type!=4&&(input.page>=disk_pages||!user_ok(p,input.buffer,4096,input.type==0)))return -1;
  unsigned i;for(i=0;i<2;i++)if(requests[i].control.phase==RQ_FREE)break;if(i==2)return CMK2_EAGAIN;
  struct rqa_request *r=&requests[i];unsigned d=i*3;
  if(rq_apply(&r->control,RQ_SUBMIT,r->control.generation,0))panic("rqa submit state");
  r->owner=pid;r->owner_generation=p->generation;r->waiter=THREADS;r->type=input.type;r->buffer=input.buffer;r->deadline=ticks+100;
  r->header.type=input.type;r->header.reserved=0;r->header.sector=(uint64_t)input.page*8;r->status=255;
  if(input.type==1)memcpy(rqa_data[i],(void *)(uintptr_t)input.buffer,4096);
  vd[d]=(struct vdesc){(uintptr_t)&r->header,sizeof r->header,1,(uint16_t)(input.type==4?d+2:d+1)};
  vd[d+1]=(struct vdesc){(uintptr_t)rqa_data[i],4096,(uint16_t)(1|(input.type==0?2:0)),(uint16_t)(d+2)};
  vd[d+2]=(struct vdesc){(uintptr_t)&r->status,1,2,0};
  va.ring[va.idx%8]=(uint16_t)d;barrier();va.idx++;barrier();async_pending++;p->io_pending=1;*notify=0;
  return (int64_t)rqa_ticket(i);
}
static int64_t rqa_status(uint64_t ticket){
  unsigned i=rqa_lookup(ticket);if(i==2)return CMK2_EINVAL;
  return requests[i].control.phase==RQ_DONE?requests[i].control.result:
    requests[i].control.phase==RQ_UNKNOWN||requests[i].control.phase==RQ_RESETTING?RQ_UNCERTAIN:CMK2_EAGAIN;
}
static int64_t async_wait(uint64_t ticket){
  unsigned i=rqa_lookup(ticket);if(i==2||requests[i].waiter!=THREADS)return CMK2_EINVAL;
  struct rqa_request *r=&requests[i];
  if(r->control.phase==RQ_UNKNOWN||r->control.phase==RQ_RESETTING)return RQ_UNCERTAIN;
  if(r->control.phase==RQ_DONE){int result=r->control.result;(void)rq_apply(&r->control,RQ_REAP,r->control.generation,0);return result;}
  r->waiter=current;uthreads[current].state=UT_IO;async_wait_blocks++;return 0;
}
static void rqa_process_exit(unsigned pid){
  for(unsigned i=0;i<2;i++)if(requests[i].owner==pid&&requests[i].control.phase==RQ_DONE)
    (void)rq_apply(&requests[i].control,RQ_REAP,requests[i].control.generation,0);
}
static int64_t rqa_syscall(struct frame *f){
  uint64_t a=f->rdi;unsigned i=rqa_lookup(a),pid=uthreads[current].process;
  switch(f->rax){
  case RQA_CANCEL:return i==2?RQ_STALE:rq_apply(&requests[i].control,RQ_CANCEL,requests[i].control.generation,0);
  case RQA_TIMEOUT:return i==2?RQ_STALE:rq_apply(&requests[i].control,RQ_TIMEOUT,requests[i].control.generation,0);
  case RQA_QUERY:return i==2?RQ_STALE:(int)requests[i].control.phase;
  case RQA_HOLD:
    if(!tasks[pid].handle_open)return CMK2_EPERM;rqa_hold=(unsigned)(a!=0);async_poll();return 0;
  case RQA_USED:return (uint16_t)(vu.idx-used_idx);
  case RQA_PENDING:return async_pending;
  case RQA_TERMINALS:return async_completions;
  case RQA_RESET_STATUS:return common_cfg[20];
  case RQA_INDICES:return ((uint64_t)va.idx<<16)|used_idx;
  case RQA_DEAD_PENDING:{unsigned n=0;for(unsigned j=0;j<2;j++)if(rq_owned(&requests[j].control)&&!tasks[requests[j].owner].alive)n++;return n;}
  case RQA_WAITERS:{unsigned n=0;for(unsigned j=0;j<2;j++)if(rq_owned(&requests[j].control)&&requests[j].waiter<THREADS&&uthreads[requests[j].waiter].state==UT_IO)n++;return n;}
  case RQA_RESET_BEGIN:
    if(!tasks[pid].handle_open||rqa_resetting)return RQ_BUSY;
    rqa_resetting=1;disk_ready=0;
    for(unsigned j=0;j<2;j++)if(rq_owned(&requests[j].control))
      (void)rq_apply(&requests[j].control,RQ_RESET_BEGIN,requests[j].control.generation,0);
    common_cfg[20]=0;barrier();return 0;
  case RQA_RESET_ACK:
    if(!tasks[pid].handle_open||!rqa_resetting)return RQ_BUSY;
    /* a=1 is an explicit teaching observer that withholds reset acknowledgement.
     * It does not simulate a stalled QEMU device or claim a physical reset delay. */
#ifndef RQA_MUTANT_ACK
    if(a||common_cfg[20])return RQ_BUSY;
#endif
    barrier();
    for(unsigned j=0;j<2;j++)if(requests[j].control.phase==RQ_RESETTING){
      (void)rq_apply(&requests[j].control,RQ_RESET_ACK,requests[j].control.generation,0);rqa_publish(j);
    }
    /* Only now may DMA storage and the ring indices be reset. Generations survive. */
    memset(vd,0,sizeof vd);memset(&va,0,sizeof va);memset(&vu,0,sizeof vu);used_idx=0;
    rqa_resetting=0;rqa_hold=0;virtio_init();return 0;
  default:return RQ_INVALID;
  }
}
