#include "rwphase.h"
#include <limits.h>
#ifndef CPF_MUTANT
#define CPF_MUTANT 0
#endif
#define BIT(t) (UINT32_C(1)<<(t))
static int valid(struct cpf_handle h){
  return h.owner&&h.owner->live&&h.task<CMX_TASKS&&(h.owner->registered&BIT(h.task))&&
    h.generation==h.owner->slot[h.task].generation&&h.generation==h.owner->kernel->task[h.task].generation&&
    h.owner->kernel->task[h.task].state!=CMX_UNUSED&&h.owner->kernel->task[h.task].state!=CMX_EXITED;
}
static uint32_t pending(const struct cmx_cpf *o,unsigned mode){
  uint32_t mask=0;
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase==CPF_PENDING&&o->slot[i].mode==mode)mask|=BIT(i);
  return mask;
}
static void terminal(struct cmx_cpf *o,unsigned t){
  struct cpf_slot *s=&o->slot[t];
  if(o->kernel->task[t].state==CMX_WAITING)cmx_latch_signal(o->kernel,&s->completion);
  s->phase=CPF_TERMINAL;s->result=CMX_OK;
}
static void grant_readers(struct cmx_cpf *o,uint32_t mask,unsigned new_phase){
  if(new_phase){o->phases++;o->last_phase=CPF_READ;}
  for(unsigned i=0;i<CMX_TASKS;i++)if(mask&BIT(i)){
    o->readers|=BIT(i);o->kernel->task[i].owned_sync++;terminal(o,i);
  }
}
static void grant_writer(struct cmx_cpf *o,uint32_t mask){
  unsigned selected=CMX_TASKS;
  for(unsigned i=0;i<CMX_TASKS;i++)if((mask&BIT(i))&&(selected==CMX_TASKS||o->slot[i].arrival<o->slot[selected].arrival))selected=i;
  o->writer=(int)selected;o->phases++;o->last_phase=CPF_WRITE;
  o->kernel->task[selected].owned_sync++;terminal(o,selected);
}
static void service(struct cmx_cpf *o){
  /* Cache cancellation/timeouts before choosing the next phase. */
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase==CPF_PENDING&&o->kernel->task[i].state!=CMX_WAITING){
    struct cpf_slot *s=&o->slot[i];s->result=o->kernel->task[i].result;s->phase=CPF_TERMINAL;
    if(s->mode==CPF_UPGRADE)o->upgrader=CMX_NONE;
  }
  if(o->closing||o->writer>=0)return;
  if(o->upgrader>=0){
    unsigned t=(unsigned)o->upgrader;
    if(o->readers==BIT(t)){
      o->readers=0;o->writer=(int)t;o->upgrader=CMX_NONE;
      o->phases++;o->last_phase=CPF_WRITE;o->slot[t].mode=CPF_WRITE;terminal(o,t);
    }
    return;
  }
  uint32_t readers=pending(o,CPF_READ),writers=pending(o,CPF_WRITE);
  if(o->readers){if(!writers&&readers)grant_readers(o,readers,0);return;}
  if(!readers&&!writers)return;
  if(readers&&(!writers||(CPF_MUTANT!=1&&o->last_phase==CPF_WRITE)))grant_readers(o,readers,1);
  else grant_writer(o,writers);
}
int cpf_init(struct cmx_cpf *o,cmx_kernel *k){
  if(!o||!k)return CMX_INVALID;
  *o=(struct cmx_cpf){.kernel=k,.writer=CMX_NONE,.upgrader=CMX_NONE,.last_phase=CPF_WRITE,.live=1};return CMX_OK;
}
int cpf_register(struct cmx_cpf *o,unsigned t,struct cpf_handle *out){
  if(!o||!o->live||o->closing||!out||t>=CMX_TASKS||(o->registered&BIT(t))||
     (o->kernel->task[t].state!=CMX_READY&&o->kernel->task[t].state!=CMX_RUNNING))return CMX_INVALID;
  o->slot[t].generation=o->kernel->task[t].generation;o->registered|=BIT(t);
  *out=(struct cpf_handle){o,t,o->slot[t].generation};return CMX_OK;
}
static int available(struct cmx_cpf *o,unsigned mode){
  if(o->writer>=0||o->upgrader>=0)return 0;
  if(mode==CPF_WRITE)return !o->readers&&!pending(o,CPF_WRITE)&&!pending(o,CPF_READ);
  return CPF_MUTANT==2||!pending(o,CPF_WRITE);
}
static int reserve(struct cpf_handle h,unsigned mode,cmx_deadline d,struct cpf_ticket *out,unsigned upgrading){
  struct cmx_cpf *o=h.owner;struct cpf_slot *s=&o->slot[h.task];
  if(o->kernel->task[h.task].state!=CMX_READY&&o->kernel->task[h.task].state!=CMX_RUNNING)return CMX_INVALID;
  if(s->serial==UINT32_MAX||o->admissions==UINT32_MAX)return CPF_RETIRED;
  if(!upgrading&&o->kernel->task[h.task].owned_sync==UINT_MAX)return CMX_FULL;
  unsigned immediate=upgrading?o->readers==BIT(h.task):(unsigned)available(o,mode);
  if(!immediate){
    s->completion.signaled=0;int result=cmx_latch_wait(o->kernel,&s->completion,h.task,d);
    if(result!=CMX_BLOCKED)return result;
  }
  s->mode=mode;s->serial++;s->arrival=++o->admissions;s->result=CMX_BLOCKED;s->phase=CPF_PENDING;
  *out=(struct cpf_ticket){h,s->serial};
  if(upgrading){
    o->upgrader=(int)h.task;
    if(immediate){o->readers=0;o->writer=(int)h.task;o->upgrader=CMX_NONE;
      o->phases++;o->last_phase=CPF_WRITE;s->mode=CPF_WRITE;terminal(o,h.task);}
  }else if(immediate){
    if(mode==CPF_READ)grant_readers(o,BIT(h.task),!o->readers);
    else grant_writer(o,BIT(h.task));
  }
  return s->phase==CPF_TERMINAL?CMX_OK:CMX_BLOCKED;
}
int cpf_begin(struct cpf_handle h,unsigned mode,cmx_deadline d,struct cpf_ticket *out){
  if(!valid(h))return CPF_STALE;
  struct cmx_cpf *o=h.owner;service(o);struct cpf_slot *s=&o->slot[h.task];
  if(!out||mode>CPF_WRITE)return CMX_INVALID;
  if(o->closing)return CMX_CLOSED;
  if(s->phase==CPF_HELD)return CMX_DEADLOCK;
  if(s->phase!=CPF_IDLE)return CMX_INVALID;
  return reserve(h,mode,d,out,0);
}
int cpf_upgrade(struct cpf_handle h,cmx_deadline d,struct cpf_ticket *out){
  if(!valid(h))return CPF_STALE;
  struct cmx_cpf *o=h.owner;service(o);struct cpf_slot *s=&o->slot[h.task];
  if(!out)return CMX_INVALID;
  if(o->closing)return CMX_CLOSED;
  if(s->phase!=CPF_HELD||s->mode!=CPF_READ||!(o->readers&BIT(h.task)))return CMX_NOT_OWNER;
  /* Retaining a read share behind an earlier writer would deadlock. */
  if(CPF_MUTANT!=3&&(o->upgrader>=0||pending(o,CPF_WRITE)))return CMX_DEADLOCK;
  return reserve(h,CPF_UPGRADE,d,out,1);
}
int cpf_result(struct cpf_ticket ticket){
  if(!valid(ticket.handle))return CPF_STALE;
  struct cmx_cpf *o=ticket.handle.owner;service(o);struct cpf_slot *s=&o->slot[ticket.handle.task];
  if(s->serial!=ticket.serial||s->phase==CPF_IDLE||s->phase==CPF_HELD)return CPF_STALE;
  if(s->phase==CPF_PENDING)return CMX_BLOCKED;
  int result=s->result;
  if(result==CMX_OK){s->phase=CPF_HELD;}
  else if(o->readers&BIT(ticket.handle.task)){s->phase=CPF_HELD;s->mode=CPF_READ;}
  else s->phase=CPF_IDLE;
  return result;
}
int cpf_cancel(struct cpf_ticket ticket){
  if(!valid(ticket.handle))return CPF_STALE;
  struct cmx_cpf *o=ticket.handle.owner;service(o);struct cpf_slot *s=&o->slot[ticket.handle.task];
  if(s->serial!=ticket.serial||s->phase==CPF_IDLE||s->phase==CPF_HELD)return CPF_STALE;
  if(s->phase==CPF_PENDING){int result=cmx_cancel(o->kernel,ticket.handle.task);if(result!=CMX_OK)return result;service(o);}
  return s->result;
}
int cpf_unlock(struct cpf_handle h){
  if(!valid(h))return CPF_STALE;
  struct cmx_cpf *o=h.owner;service(o);struct cpf_slot *s=&o->slot[h.task];
  if(s->phase!=CPF_HELD)return CMX_NOT_OWNER;
  if(o->writer==(int)h.task)o->writer=CMX_NONE;
  else if(o->readers&BIT(h.task))o->readers&=~BIT(h.task);
  else return CMX_NOT_OWNER;
  o->kernel->task[h.task].owned_sync--;s->phase=CPF_IDLE;service(o);return CMX_OK;
}
int cpf_close(struct cmx_cpf *o){
  if(!o||!o->live)return CMX_INVALID;
  service(o);if(o->writer>=0||o->readers)return CMX_NOT_OWNER;
  o->closing=1;
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase==CPF_PENDING)(void)cmx_cancel(o->kernel,i);
  service(o);return CMX_OK;
}
int cpf_release(struct cpf_handle *h){
  if(!h||!valid(*h))return CPF_STALE;
  struct cmx_cpf *o=h->owner;service(o);
  if(!o->closing||o->slot[h->task].phase!=CPF_IDLE)return CMX_BLOCKED;
  o->registered&=~BIT(h->task);*h=(struct cpf_handle){0};return CMX_OK;
}
int cpf_destroy(struct cmx_cpf *o){
  if(!o||!o->live)return CMX_INVALID;
  service(o);if(!o->closing||o->registered)return CMX_BLOCKED;
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase!=CPF_IDLE)return CMX_BLOCKED;
  o->live=0;return CMX_OK;
}
