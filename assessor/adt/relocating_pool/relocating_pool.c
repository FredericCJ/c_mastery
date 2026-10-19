#include "relocating_pool.h"
#ifndef ADP_MUTATION
#define ADP_MUTATION 0
#endif
struct AdpBlock {AdpSlot slots[4];};
AdpStatus adp_ids_init(AdpIds *s,uint64_t first){if(!s||!first)return ADP_INVALID;*s=(AdpIds){first,0};return ADP_OK;}
AdpStatus adp_issue(AdpIds *s,AdpId *out) {
  if(!s||!out)return ADP_INVALID;
  if(s->exhausted)return ADP_FULL;
  *out=(AdpId){s->next};if(s->next==UINT64_MAX)s->exhausted=1;else ++s->next;return ADP_OK;
}
AdpStatus adp_init(AdpPool *p,AdpId id,AdpOps ops,uint64_t gen) {
  if(!p||!id.value||!gen||!ops.allocate||!ops.deallocate)return ADP_INVALID;
  struct AdpBlock *block=ops.allocate(ops.context,sizeof *block);
  if(!block)return ADP_NOMEM;
  for(unsigned i=0;i<4;++i)block->slots[i]=(AdpSlot){gen,ADP_FREE,0};
  if(pthread_mutex_init(&p->mutex,NULL)!=0){ops.deallocate(ops.context,block);return ADP_SYNC;}
  p->identity=id;p->block=block;p->ops=ops;return ADP_OK;
}
static AdpStatus lock(AdpPool *p) {if(!p)return ADP_INVALID;return pthread_mutex_lock(&p->mutex)==0?ADP_OK:ADP_SYNC;}
static AdpStatus done(AdpPool *p,AdpStatus status){(void)pthread_mutex_unlock(&p->mutex);return status;}
static int valid(const AdpPool *p,AdpHandle h) {
  return (ADP_MUTATION==1||h.pool.value==p->identity.value)&&h.index<4&&
    p->block->slots[h.index].state==ADP_LIVE&&
    (ADP_MUTATION==2||h.generation==p->block->slots[h.index].generation);
}
AdpStatus adp_acquire(AdpPool *p,uint32_t value,AdpHandle *out) {
  if(!out)return ADP_INVALID;
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  for(unsigned i=0;i<4;++i)if(p->block->slots[i].state==ADP_FREE) {
    p->block->slots[i].state=ADP_LIVE;p->block->slots[i].value=value;
    *out=(AdpHandle){p->identity,i,p->block->slots[i].generation};return done(p,ADP_OK);
  }
  return done(p,ADP_FULL);
}
AdpStatus adp_get(AdpPool *p,AdpHandle h,uint32_t *out) {
  if(!out)return ADP_INVALID;
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(ADP_MUTATION==7)*out=0;
  if(!p->block)return done(p,ADP_CLOSED);
  if(!valid(p,h))return done(p,ADP_STALE);
  *out=p->block->slots[h.index].value;return done(p,ADP_OK);
}
AdpStatus adp_release(AdpPool *p,AdpHandle h) {
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  if(!valid(p,h))return done(p,ADP_STALE);
  AdpSlot *slot=&p->block->slots[h.index];slot->value=0;
  if(slot->generation==UINT64_MAX) {
    if(ADP_MUTATION==3){slot->generation=1;slot->state=ADP_FREE;}
    else slot->state=ADP_RETIRED;
  } else {++slot->generation;if(ADP_MUTATION==4&&slot->generation<UINT64_MAX)++slot->generation;slot->state=ADP_FREE;}
  return done(p,ADP_OK);
}
AdpStatus adp_snapshot(AdpPool *p,AdpSnapshot *out) {
  if(!out)return ADP_INVALID;
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  AdpSnapshot next;next.identity=p->identity;
  for(unsigned i=0;i<4;++i)next.slots[i]=p->block->slots[i];
  *out=next;return done(p,ADP_OK);
}
AdpStatus adp_compact(AdpPool *p) {
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  struct AdpBlock *next=p->ops.allocate(p->ops.context,sizeof *next);
  if(!next)return done(p,ADP_NOMEM);
  for(unsigned i=0;i<4;++i){next->slots[i]=p->block->slots[i];if(ADP_MUTATION==6)next->slots[i].value=0;}
  struct AdpBlock *old=p->block;p->block=next;p->ops.deallocate(p->ops.context,old);
  return done(p,ADP_OK);
}
AdpStatus adp_move(AdpPool *destination,AdpPool *source) {
  if(!destination||!source||destination==source)return ADP_INVALID;
  /* Both controls are exclusively accessible here; never copy the mutex. */
  if(!source->block)return ADP_CLOSED;
  if(pthread_mutex_init(&destination->mutex,NULL)!=0)return ADP_SYNC;
  destination->identity=source->identity;
  if(ADP_MUTATION==5)destination->identity.value^=UINT64_C(1);
  destination->block=source->block;destination->ops=source->ops;
  source->block=NULL;return ADP_OK;
}
AdpStatus adp_close(AdpPool *p) {
  AdpStatus s=lock(p);if(s!=ADP_OK)return s;
  if(p->block){struct AdpBlock *old=p->block;p->block=NULL;if(ADP_MUTATION!=8)p->ops.deallocate(p->ops.context,old);}
  return done(p,ADP_OK);
}
AdpStatus adp_destroy(AdpPool *p) {
  if(!p)return ADP_INVALID;
  if(p->block)return ADP_STATE;
  return pthread_mutex_destroy(&p->mutex)==0?ADP_OK:ADP_SYNC;
}
