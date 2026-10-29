#include "rwlock.h"
#include <limits.h>
#ifndef CDR_MUTANT
#define CDR_MUTANT 0
#endif
#define BIT(t) (UINT32_C(1)<<(t))
static int valid(struct cdr_handle h){
  return h.owner&&h.owner->live&&h.task<CMX_TASKS&&(h.owner->registered&BIT(h.task))&&
    h.generation==h.owner->slot[h.task].generation&&h.generation==h.owner->kernel->task[h.task].generation&&
    h.owner->kernel->task[h.task].state!=CMX_UNUSED&&h.owner->kernel->task[h.task].state!=CMX_EXITED;
}
static void settle(struct cd_rw *o){
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase==CDR_PENDING&&o->kernel->task[i].state!=CMX_WAITING){
    o->slot[i].result=o->kernel->task[i].result;o->slot[i].phase=CDR_TERMINAL;
  }
}
static unsigned readers_reserved(const struct cd_rw *o){
  unsigned count=0;
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase!=CDR_IDLE&&o->slot[i].mode==CDR_READ)count++;
  return count;
}
int cdr_init(struct cd_rw *o,cmx_kernel *k,unsigned limit){
  if(!o||!k||!limit||limit>CMX_TASKS)return CMX_INVALID;
  *o=(struct cd_rw){.kernel=k,.read_limit=limit,.live=1};cmx_rw_init(&o->raw);return CMX_OK;
}
int cdr_register(struct cd_rw *o,unsigned t,struct cdr_handle *out){
  if(!o||!o->live||o->closing||!out||t>=CMX_TASKS||(o->registered&BIT(t))||
     (o->kernel->task[t].state!=CMX_READY&&o->kernel->task[t].state!=CMX_RUNNING))return CMX_INVALID;
  o->slot[t].generation=o->kernel->task[t].generation;o->registered|=BIT(t);
  *out=(struct cdr_handle){o,t,o->slot[t].generation};return CMX_OK;
}
int cdr_begin(struct cdr_handle h,unsigned mode,cmx_deadline d,struct cdr_ticket *out){
  if(!valid(h))return CDR_STALE;
  struct cd_rw *o=h.owner;settle(o);struct cdr_slot *s=&o->slot[h.task];
  if(!out||mode>CDR_WRITE)return CMX_INVALID;
  if(o->closing)return CMX_CLOSED;
  if(s->phase==CDR_HELD)return CMX_DEADLOCK;
  if(s->phase!=CDR_IDLE)return CMX_INVALID;
  if(s->serial==UINT32_MAX)return CDR_RETIRED;
  if(o->kernel->task[h.task].owned_sync==UINT_MAX)return CMX_FULL;
  if(mode==CDR_READ&&CDR_MUTANT!=2&&readers_reserved(o)>=o->read_limit)return CMX_FULL;
  int result;
  if(CDR_MUTANT==1&&mode==CDR_READ&&o->raw.writer<0){
    /* Explicit reader-preference control: bypass the actual pending writer. */
    o->raw.readers|=BIT(h.task);o->kernel->task[h.task].owned_sync++;result=CMX_OK;
  }else result=mode==CDR_READ?cmx_rw_read(o->kernel,&o->raw,h.task,d):cmx_rw_write(o->kernel,&o->raw,h.task,d);
  if(result!=CMX_OK&&result!=CMX_BLOCKED)return result;
  s->mode=mode;s->result=result;s->phase=result==CMX_OK?CDR_TERMINAL:CDR_PENDING;s->serial++;
  *out=(struct cdr_ticket){h,s->serial};return result;
}
int cdr_result(struct cdr_ticket ticket){
  if(!valid(ticket.handle))return CDR_STALE;
  struct cd_rw *o=ticket.handle.owner;settle(o);struct cdr_slot *s=&o->slot[ticket.handle.task];
  if(s->serial!=ticket.serial||s->phase==CDR_IDLE||s->phase==CDR_HELD)return CDR_STALE;
  if(s->phase==CDR_PENDING)return CMX_BLOCKED;
  int result=s->result;s->phase=result==CMX_OK?CDR_HELD:CDR_IDLE;return result;
}
int cdr_cancel(struct cdr_ticket ticket){
  if(!valid(ticket.handle))return CDR_STALE;
  struct cd_rw *o=ticket.handle.owner;settle(o);struct cdr_slot *s=&o->slot[ticket.handle.task];
  if(s->serial!=ticket.serial||s->phase==CDR_IDLE||s->phase==CDR_HELD)return CDR_STALE;
  if(s->phase==CDR_PENDING){int r=cmx_cancel(o->kernel,ticket.handle.task);if(r!=CMX_OK)return r;settle(o);}
  return s->result;
}
int cdr_unlock(struct cdr_handle h){
  if(!valid(h))return CDR_STALE;
  struct cd_rw *o=h.owner;settle(o);struct cdr_slot *s=&o->slot[h.task];
  if(s->phase!=CDR_HELD)return CMX_NOT_OWNER;
  int result=cmx_rw_unlock(o->kernel,&o->raw,h.task);
  if(result==CMX_OK)s->phase=CDR_IDLE;
  settle(o);return result;
}
int cdr_close(struct cd_rw *o){
  if(!o||!o->live)return CMX_INVALID;
  settle(o);int result=cmx_rw_close(o->kernel,&o->raw);
  if(result==CMX_OK){o->closing=1;settle(o);}return result;
}
int cdr_release(struct cdr_handle *h){
  if(!h||!valid(*h))return CDR_STALE;
  struct cd_rw *o=h->owner;settle(o);
  if(!o->closing||o->slot[h->task].phase!=CDR_IDLE)return CMX_BLOCKED;
  o->registered&=~BIT(h->task);*h=(struct cdr_handle){0};return CMX_OK;
}
int cdr_destroy(struct cd_rw *o){
  if(!o||!o->live)return CMX_INVALID;
  settle(o);if(!o->closing||o->registered)return CMX_BLOCKED;
  for(unsigned i=0;i<CMX_TASKS;i++)if(o->slot[i].phase!=CDR_IDLE)return CMX_BLOCKED;
  o->live=0;return CMX_OK;
}
