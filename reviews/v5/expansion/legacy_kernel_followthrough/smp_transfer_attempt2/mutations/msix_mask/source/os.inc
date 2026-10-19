/* Called under the shared kernel lock. User tasks execute concurrently;
 * object transitions and page-table ownership serialize here. */
#define CHANNELS 8
struct shared_object { unsigned frame,generation,owner,allowed,open; };
static struct shared_object shared_objects[4];
static void shared_close(unsigned id){struct shared_object *s=&shared_objects[id];s->open=0;if(!page_refs[s->frame/4096])panic("shared reference");if(!--page_refs[s->frame/4096]&&cmk_frame_put(&frames,s->frame))panic("shared release");}
static unsigned shared_lookup(uint64_t handle,unsigned pid){for(unsigned i=0;i<4;i++)if(handle<=UINT32_MAX&&shared_objects[i].open&&(shared_objects[i].allowed&(1u<<pid))&&cmk_handle_valid((uint32_t)handle,i,shared_objects[i].generation,1))return i;return 4;}
struct endpoint { struct cmk2_channel q; unsigned owner,allowed,open,generation; };
static struct endpoint endpoints[CHANNELS];
static void close_endpoint(unsigned e){
  struct endpoint *p=&endpoints[e];p->open=2;cmk2_channel_close(&p->q);
  for(unsigned i=0;i<thread_count;i++)if((uthreads[i].state==UT_SEND||uthreads[i].state==UT_RECV)&&uthreads[i].wait==e){
    struct cmk2_message m;int drain=uthreads[i].state==UT_RECV&&cmk2_channel_receive(&p->q,&m)==CMK2_OK;
    uthreads[i].frame->rax=drain?m.value:(uint64_t)CMK2_ECLOSED;uthreads[i].state=UT_READY;
  }
}
static void process_exit(unsigned id,int status){
  struct task *p=&tasks[id];p->status=status;p->alive=0;p->handle_open=0;if(p->parent==TASKS&&status&&!(id==3&&p->generation==1))primary_failure=1;
  if(io_request.state==2&&io_request.owner==id)io_request.state=0;
  for(unsigned child=0;child<task_count;child++)if(tasks[child].parent==id&&tasks[child].parent_generation==p->generation)tasks[child].waited=1;
  for(unsigned e=0;e<CHANNELS;e++)if(endpoints[e].open&&endpoints[e].owner==id){close_endpoint(e);endpoints[e].owner=TASKS;}
  for(unsigned e=0;e<4;e++)if(shared_objects[e].open&&shared_objects[e].owner==id)shared_close(e);
  for(unsigned i=0;i<thread_count;i++){
    if(uthreads[i].process==id&&uthreads[i].state!=UT_UNUSED)uthreads[i].state=UT_DEAD;

  }
  for(unsigned e=0;e<CHANNELS;e++)endpoints[e].allowed&=~(1u<<id);
  for(unsigned e=0;e<4;e++)shared_objects[e].allowed&=~(1u<<id);
  /* Owner death closes publication. An unreachable queue is reclaimed;
   * surviving grantees may drain and destroy an orphaned closed queue. */
  for(unsigned e=0;e<CHANNELS;e++)if(endpoints[e].open){unsigned reachable=0;for(unsigned j=0;j<task_count;j++)if(tasks[j].alive&&(endpoints[e].allowed&(1u<<j)))reachable=1;if(!reachable)endpoints[e].open=0;}
  for(unsigned c=0;c<cmk2_cpu_count();c++)if(c!=cmk2_cpu_index()&&currents[c]<thread_count&&uthreads[currents[c]].process==id)cmk2_wake_cpu(c);
}
static unsigned endpoint_lookup(uint64_t h,unsigned pid){
  if(h>UINT32_MAX)return CHANNELS;
  for(unsigned e=0;e<CHANNELS;e++)if(endpoints[e].open &&
    cmk_handle_valid((uint32_t)h,e,endpoints[e].generation,1)&& (endpoints[e].allowed&(1u<<pid)))return e;
  return CHANNELS;
}
static int64_t fork_single(struct frame *f){
  unsigned pid=uthreads[current].process,live=0;
  for(unsigned i=0;i<thread_count;i++)if(uthreads[i].process==pid&&uthreads[i].state!=UT_UNUSED&&uthreads[i].state!=UT_DEAD)live++;
  if(live!=1)return CMK2_EINVAL;
  unsigned child=task_slot();if(child==TASKS)return CMK2_EAGAIN;unsigned generation=tasks[child].generation+1;unsigned tid=new_thread(child,f);if(tid==THREADS)return CMK2_EAGAIN;
  struct task *p=&tasks[pid],*q=&tasks[child];memcpy(q,p,sizeof *q);q->parent=pid;q->parent_generation=p->generation;q->waited=0;q->quiescent=0;q->reaped=0;q->status=0;q->handle_open=0;q->generation=generation;q->io_pending=0;q->handle_generation=1;q->name="cow-child";
  q->pml4[0]=(uintptr_t)q->pdpt|7;q->pdpt[1]=(uintptr_t)q->pd|7;q->pd[0]=(uintptr_t)q->pt|7;
  for(unsigned i=0;i<512;i++)if(p->pt[i]&1){
    unsigned frame=(unsigned)((p->pt[i]&UINT64_C(0x000ffffffffff000))/4096);
    if(!page_refs[frame]||page_refs[frame]==65535)panic("fork references");page_refs[frame]++;
    if((p->pt[i]&2)&&!(p->pt[i]&PTE_SHARED)){p->pt[i]=(p->pt[i]&~UINT64_C(2))|PTE_COW;q->pt[i]=p->pt[i];}
  }
  for(unsigned e=0;e<CHANNELS;e++)if(endpoints[e].open&&(endpoints[e].allowed&(1u<<pid)))endpoints[e].allowed|=1u<<child;
  for(unsigned e=0;e<4;e++)if(shared_objects[e].open&&(shared_objects[e].allowed&(1u<<pid)))shared_objects[e].allowed|=1u<<child;
  uthreads[tid].frame->rax=0;if(child==task_count)task_count++;
  uint64_t cr3=(uintptr_t)p->pml4;__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");cmk2_flush_remote();
  return cmk_handle(child,generation);
}
static int64_t syscall_v2(struct frame *f){
  uint64_t a=f->rdi,b=f->rsi,c=f->rdx;unsigned pid=uthreads[current].process;struct task *p=&tasks[pid];
  switch(f->rax){
  case CMK2_TLB_SYNC:cmk2_flush_remote();return 0;
  case CMK2_CPU_INFO:return ((int64_t)cmk2_cpu_count()<<32)|cmk2_cpu_index();
  case CMK2_SET_AFFINITY:if(!a||(a&~((1u<<cmk2_cpu_count())-1)))return CMK2_EINVAL;uthreads[current].affinity=(unsigned)a;uthreads[current].state=UT_READY;return 0;
  case CMK2_SPAWN:{
    if(!b||b>23||c>1||!user_ok(p,a,b,0) )return CMK2_EINVAL;if(c&&!p->handle_open)return CMK2_EPERM;
    char name[24];memcpy(name,(const void *)(uintptr_t)a,(size_t)b);name[b]=0;
    unsigned child=task_slot();if(child==TASKS)return CMK2_EAGAIN;int loaded=load_task(name,(int)c);if(loaded)return loaded==-2?CMK2_EAGAIN:CMK2_EINVAL;
    tasks[child].name="spawned";tasks[child].parent=pid;tasks[child].parent_generation=p->generation;return cmk_handle(child,tasks[child].generation);
  }
  case CMK2_CHANNEL_DESTROY:{unsigned e=endpoint_lookup(a,pid);if(e==CHANNELS||(endpoints[e].owner!=pid&&endpoints[e].owner<TASKS&&tasks[endpoints[e].owner].alive)||endpoints[e].open!=2||endpoints[e].q.count)return CMK2_EINVAL;endpoints[e].open=0;return 0;}

  case CMK2_BLOCK_STATUS:if(!io_request.state||io_request.owner!=pid||a!=io_request.ticket)return CMK2_EINVAL;return io_request.state==1?CMK2_EAGAIN:io_request.result;
  case CMK2_BLOCK_SUBMIT:return async_submit(a,b);
  case CMK2_BLOCK_WAIT:return async_wait(a);
  case CMK2_SHM_CREATE:
    for(unsigned i=0;i<4;i++)if(!shared_objects[i].open&&shared_objects[i].generation<65535){unsigned frame=cmk_frame_take(&frames);if(!frame)return CMK2_EAGAIN;memset((void *)(uintptr_t)frame,0,4096);page_refs[frame/4096]=1;struct shared_object *s=&shared_objects[i];s->frame=frame;s->generation++;s->owner=pid;s->allowed=1u<<pid;s->open=1;return cmk_handle(i,s->generation);}return CMK2_EAGAIN;
  case CMK2_SHM_MAP:{unsigned i=shared_lookup(a,pid);if(i==4)return CMK2_EPERM;for(unsigned j=256;j<479;j++)if(!p->pt[j]){p->pt[j]=(uint64_t)shared_objects[i].frame|7|PTE_NX|PTE_SHARED;page_refs[shared_objects[i].frame/4096]++;return CMK_USER_BASE+(uint64_t)j*4096;}return CMK2_EAGAIN;}
  case CMK2_SHM_CLOSE:{unsigned i=shared_lookup(a,pid);if(i==4||shared_objects[i].owner!=pid)return CMK2_EPERM;shared_close(i);return 0;}
  case CMK2_THREAD_CREATE:{
    if(!user_ok(p,a,1,0)||p->pt[(a-CMK_USER_BASE)/4096]&PTE_NX || b<16 || (b&15)||!user_ok(p,b-16,16,1))return CMK2_EINVAL;
    unsigned id=new_thread(pid,0);if(id==THREADS)return CMK2_EAGAIN;
    struct frame *r=uthreads[id].frame;r->rip=a;r->rsp=b-8;r->rdi=c;r->cs=27;r->ss=35;r->flags=0x202;
    return cmk_handle(id,uthreads[id].generation);
  }
  case CMK2_THREAD_EXIT:{
    struct uthread *u=&uthreads[current];u->status=(int)a;u->state=UT_DEAD;
    if(u->joiner<THREADS){struct uthread *j=&uthreads[u->joiner];j->frame->rax=(uint64_t)(int64_t)u->status;j->state=UT_READY;u->state=UT_UNUSED;}
    unsigned live=0;for(unsigned i=0;i<thread_count;i++)if(uthreads[i].process==pid&&uthreads[i].state!=UT_UNUSED&&uthreads[i].state!=UT_DEAD)live++;
    if(!live)process_exit(pid,(int)a);return 0;
  }
  case CMK2_THREAD_JOIN:{
    if(a>UINT32_MAX)return CMK2_EINVAL;unsigned id=(unsigned)(a&65535);if(!id||--id>=thread_count||id==current)return CMK2_EINVAL;
    struct uthread *u=&uthreads[id];if(u->process!=pid||u->state==UT_UNUSED||u->joiner!=THREADS||!cmk_handle_valid((uint32_t)a,id,u->generation,1))return CMK2_EINVAL;
    if(u->state==UT_DEAD){int status=u->status;u->state=UT_UNUSED;return status;}
    u->joiner=current;uthreads[current].state=UT_JOIN;uthreads[current].wait=id;return 0;
  }
  case CMK2_CHANNEL_CREATE:
    for(unsigned e=0;e<CHANNELS;e++)if(!endpoints[e].open&&endpoints[e].generation<65535){struct endpoint *q=&endpoints[e];q->generation++;q->owner=pid;q->allowed=1u<<pid;q->open=1;memset(&q->q,0,sizeof q->q);return cmk_handle(e,q->generation);}return CMK2_EAGAIN;
  case CMK2_CHANNEL_GRANT:{unsigned e=endpoint_lookup(a,pid),child=(unsigned)(b&65535);if(e==CHANNELS||endpoints[e].owner!=pid||b>UINT32_MAX||!child||--child>=task_count||!tasks[child].alive||!cmk_handle_valid((uint32_t)b,child,tasks[child].generation,1))return CMK2_EPERM;endpoints[e].allowed|=1u<<child;return 0;}
  case CMK2_CHANNEL_CLOSE:{unsigned e=endpoint_lookup(a,pid);if(e==CHANNELS||endpoints[e].owner!=pid)return CMK2_EPERM;close_endpoint(e);return 0;}
  case CMK2_CHANNEL_SEND:{
    unsigned e=endpoint_lookup(a,pid);if(e==CHANNELS)return CMK2_EPERM;if(endpoints[e].open!=1)return CMK2_ECLOSED;if(b>INT32_MAX||c>1)return CMK2_EINVAL;
    for(unsigned i=0;i<thread_count;i++)if(uthreads[i].state==UT_RECV&&uthreads[i].wait==e){uthreads[i].frame->rax=b;uthreads[i].state=UT_READY;return 0;}
    int r=cmk2_channel_send(&endpoints[e].q,(struct cmk2_message){0,(uint32_t)b});
    if(r==CMK2_OK)return 0;if(r!=CMK2_FULL)return CMK2_ECLOSED;if(c)return CMK2_EAGAIN;
    uthreads[current].wait=e;uthreads[current].value=(unsigned)b;uthreads[current].state=UT_SEND;return 0;
  }
  case CMK2_CHANNEL_RECV:{
    unsigned e=endpoint_lookup(a,pid);if(e==CHANNELS)return CMK2_EPERM;if(c>1)return CMK2_EINVAL;struct cmk2_message m;
    int r=cmk2_channel_receive(&endpoints[e].q,&m);
    if(r==CMK2_OK){for(unsigned i=0;i<thread_count;i++)if(uthreads[i].state==UT_SEND&&uthreads[i].wait==e){
      if(cmk2_channel_send(&endpoints[e].q,(struct cmk2_message){0,uthreads[i].value}))panic("sender wake queue");uthreads[i].frame->rax=0;uthreads[i].state=UT_READY;break;}return m.value;}
    if(r!=CMK2_EMPTY)return CMK2_ECLOSED;if(c)return CMK2_EAGAIN;uthreads[current].wait=e;uthreads[current].state=UT_RECV;return 0;
  }
  case CMK2_RESERVE_ZERO:{
    if(!a||a>16)return CMK2_EINVAL;
    for(unsigned first=256;first+a<=479;first++){unsigned j;for(j=0;j<a;j++)if(p->pt[first+j])break;if(j!=a)continue;
      for(j=0;j<a;j++)p->pt[first+j]=PTE_ZERO|6|PTE_NX;return CMK_USER_BASE+(uint64_t)first*4096;}return CMK2_EAGAIN;
  }
  case CMK2_FORK_SINGLE:return fork_single(f);
  case CMK2_WAIT_PROCESS:{
    unsigned id=(unsigned)(a&65535);if(a>UINT32_MAX||!id||--id>=task_count||!cmk_handle_valid((uint32_t)a,id,tasks[id].generation,1)||tasks[id].parent!=pid||tasks[id].parent_generation!=p->generation||tasks[id].waited)return CMK2_EINVAL;
    if(tasks[id].quiescent){tasks[id].waited=1;return tasks[id].status;}
    for(unsigned i=0;i<thread_count;i++)if(uthreads[i].state==UT_WAIT&&uthreads[i].wait==id)return CMK2_EINVAL;
    uthreads[current].state=UT_WAIT;uthreads[current].wait=id;return 0;
  }
  default:return CMK2_EINVAL;
  }
}
static int page_fault_v2(struct frame *f){
  uint64_t a;__asm__ volatile("mov %%cr2,%0":"=r"(a));if(a<CMK_USER_BASE||a>=CMK_USER_LIMIT)return 0;
  struct task *p=&tasks[uthreads[current].process];unsigned i=(unsigned)((a-CMK_USER_BASE)/4096);uint64_t e=p->pt[i];
  if((!(f->error&1)&&(e&PTE_ZERO)&&!(f->error&16)) || ((f->error&7)==7&&(e&PTE_COW)))return materialize(p,i,!!(f->error&2))>0;
  /* Another CPU may have resolved the same pending fault already. */
  if((f->error&4)&&!(f->error&8)&&(e&5)==5&&(!(f->error&2)||(e&2))&&(!(f->error&16)||!(e&PTE_NX))){__asm__ volatile("invlpg (%0)"::"r"(a):"memory");return 1;}
  return 0;
}
