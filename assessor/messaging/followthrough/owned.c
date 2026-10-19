#include "owned.h"
#include <limits.h>
#ifndef MFC_MUTANT
#define MFC_MUTANT 0
#endif
static bool valid_value(const MfoValue *v) { return v && (!v->data || v->drop); }
void mfo_drop(MfoValue *value) {
  MfoValue old=*value;*value=(MfoValue){0};
  if(old.drop)old.drop(old.data);
}
MsgStatus mfc_init(MfcHub *h, size_t a, size_t b) {
  if (!h || a>4 || b>4) return MSG_INVALID;
  *h=(MfcHub){0};
  if(pthread_mutex_init(&h->records_mutex,NULL)) return MSG_FAILED;
  MsgStatus status=msg_hub_init(&h->hub,a,b);
  if(status!=MSG_OK) pthread_mutex_destroy(&h->records_mutex);
  return status;
}
MsgStatus mfc_send(MfcHub *h,unsigned channel,MfoValue *value,bool wait,
                   MfcAfterSubmit hook,void *ctx) {
  if(!h || channel>1 || !valid_value(value)) return MSG_INVALID;
  pthread_mutex_lock(&h->records_mutex);
  if(h->retired || h->active==UINT_MAX) {
    MsgStatus s=h->retired ? MSG_CLOSED : MSG_FULL;
    pthread_mutex_unlock(&h->records_mutex);return s;
  }
  unsigned slot=0;while(slot<8 && h->records[slot].used) slot++;
  if(slot==8) {pthread_mutex_unlock(&h->records_mutex);return MSG_FULL;}
  h->records[slot]=(MfcRecord){.value=*value,.used=true};h->active++;
  pthread_mutex_unlock(&h->records_mutex);
  MsgStatus status=msg_send(&h->hub,channel,slot,wait);
  if(hook) hook(ctx);
  pthread_mutex_lock(&h->records_mutex);
  MfcRecord *record=&h->records[slot];
  if(status==MSG_OK) {
    *value=(MfoValue){0};record->send_done=true;
    if(record->delivered) record->used=false;
  } else {
    /* Imported failed submission cannot have been accepted by a receiver. */
    record->used=false;record->value=(MfoValue){0};
    if(MFC_MUTANT==1) *value=(MfoValue){0};
  }
  h->active--;pthread_mutex_unlock(&h->records_mutex);
  return status;
}
MsgStatus mfc_select(MfcHub *h,unsigned mask,bool wait,unsigned *channel,MfoValue *out) {
  if(!h || !channel || !out) return MSG_INVALID;
  pthread_mutex_lock(&h->records_mutex);
  if(h->retired || h->active==UINT_MAX) {
    MsgStatus status=h->retired ? MSG_CLOSED : MSG_FULL;
    pthread_mutex_unlock(&h->records_mutex);return status;
  }
  h->active++;pthread_mutex_unlock(&h->records_mutex);
  unsigned selected,slot;
  MsgStatus status=msg_select(&h->hub,mask,wait,&selected,&slot);
  pthread_mutex_lock(&h->records_mutex);
  if(status==MSG_OK) {
    MfcRecord *record=&h->records[slot];
    *out=record->value;record->value=(MfoValue){0};record->delivered=true;
    if(record->send_done || MFC_MUTANT==2) record->used=false;
    *channel=selected;
  }
  h->active--;pthread_mutex_unlock(&h->records_mutex);
  return status;
}
MsgStatus mfc_close(MfcHub *h,unsigned channel) {return h ? msg_close(&h->hub,channel) : MSG_INVALID;}
void mfc_cancel(MfcHub *h) {msg_hub_cancel(&h->hub);}
MsgStatus mfc_destroy(MfcHub *h) {
  if(!h) return MSG_INVALID;
  MfoValue values[8];unsigned count=0;
  pthread_mutex_lock(&h->records_mutex);
  if(h->active) {pthread_mutex_unlock(&h->records_mutex);return MSG_BUSY;}
  h->retired=true;
  for(unsigned i=0;i<8;i++) if(h->records[i].used) {
    values[count++]=h->records[i].value;h->records[i].value=(MfoValue){0};h->records[i].used=false;
  }
  pthread_mutex_unlock(&h->records_mutex);
  msg_hub_cancel(&h->hub);
  for(unsigned i=0;i<count;i++) mfo_drop(&values[i]);
  msg_hub_destroy(&h->hub);pthread_mutex_destroy(&h->records_mutex);
  return MSG_OK;
}
static bool actor_deliver(void *arg,unsigned slot) {
  MfaActor *a=arg;MfoValue value=a->records[slot].value;
  a->records[slot]=(MfaRecord){0};
  bool keep=a->handler(a->ctx,&value);
  mfo_drop(&value);
  if(!keep) for(unsigned i=0;i<5;i++) if(a->records[i].used) {
    MfoValue pending=a->records[i].value;a->records[i]=(MfaRecord){0};
    if(MFC_MUTANT!=3) mfo_drop(&pending);
  }
  return keep;
}
MsgStatus mfa_init(MfaActor *a,MfaHandler handler,void *ctx,MfaDeadLetter dead,void *dc) {
  if(!a || !handler || !dead) return MSG_INVALID;
  *a=(MfaActor){.handler=handler,.dead=dead,.ctx=ctx,.dead_ctx=dc};
  msg_actor_init(&a->actor,actor_deliver,a);return MSG_OK;
}
MsgStatus mfa_send(MfaActor *a,uint64_t generation,MfoValue *value) {
  if(!a || !valid_value(value)) return MSG_INVALID;
  if(generation!=a->actor.generation && MFC_MUTANT!=4) {
    MfoValue old=*value;*value=(MfoValue){0};a->dead(a->dead_ctx,generation,old);return MSG_STALE;
  }
  unsigned slot=0;while(slot<5 && a->records[slot].used) slot++;
  if(slot==5) return MSG_FULL;
  MsgStatus status=msg_actor_send(&a->actor,MFC_MUTANT==4?a->actor.generation:generation,slot);
  if(status==MSG_OK) {a->records[slot]=(MfaRecord){*value,true};*value=(MfoValue){0};}
  return status;
}
MsgStatus mfa_step(MfaActor *a) {return a ? msg_actor_step(&a->actor) : MSG_INVALID;}
MsgStatus mfa_restart(MfaActor *a) {return a ? msg_actor_restart(&a->actor) : MSG_INVALID;}
MsgStatus mfa_destroy(MfaActor *a) {
  if(!a) return MSG_INVALID;
  if(a->actor.stepping) return MSG_BUSY;
  a->actor.alive=false;a->actor.count=0;
  for(unsigned i=0;i<5;i++) if(a->records[i].used) {
    MfoValue value=a->records[i].value;a->records[i]=(MfaRecord){0};mfo_drop(&value);
  }
  return MSG_OK;
}
