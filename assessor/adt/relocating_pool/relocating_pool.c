#include "relocating_pool.h"
#ifndef ADP_MUTATION
#define ADP_MUTATION 0
#endif
struct adp_block {struct adp_slot slots[4];};
enum adp_status adp_ids_init(struct adp_ids *s,uint64_t first){if(!s||!first)return ADP_INVALID;*s=(struct adp_ids){first,0};return ADP_OK;}
enum adp_status adp_issue(struct adp_ids *s,struct adp_id *out) {
  if(!s||!out)return ADP_INVALID;
  if(s->exhausted)return ADP_FULL;
  *out=(struct adp_id){s->next};if(s->next==UINT64_MAX)s->exhausted=1;else ++s->next;return ADP_OK;
}
enum adp_status adp_init(struct adp_pool *p,struct adp_id id,struct adp_ops ops,uint64_t gen) {
  if(!p||!id.value||!gen||!ops.allocate||!ops.deallocate)return ADP_INVALID;
  struct adp_block *block=ops.allocate(ops.context,sizeof *block);
  if(!block)return ADP_NOMEM;
  for(unsigned i=0;i<4;++i)block->slots[i]=(struct adp_slot){gen,ADP_FREE,0};
  if(pthread_mutex_init(&p->mutex,NULL)!=0){ops.deallocate(ops.context,block);return ADP_SYNC;}
  p->identity=id;p->block=block;p->ops=ops;return ADP_OK;
}
static enum adp_status lock(struct adp_pool *p) {if(!p)return ADP_INVALID;return pthread_mutex_lock(&p->mutex)==0?ADP_OK:ADP_SYNC;}
static enum adp_status done(struct adp_pool *p,enum adp_status status){(void)pthread_mutex_unlock(&p->mutex);return status;}
static int valid(const struct adp_pool *p,struct adp_handle h) {
  return (ADP_MUTATION==1||h.pool.value==p->identity.value)&&h.index<4&&
    p->block->slots[h.index].state==ADP_LIVE&&
    (ADP_MUTATION==2||h.generation==p->block->slots[h.index].generation);
}
enum adp_status adp_acquire(struct adp_pool *p,uint32_t value,struct adp_handle *out) {
  if(!out)return ADP_INVALID;
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  for(unsigned i=0;i<4;++i)if(p->block->slots[i].state==ADP_FREE) {
    p->block->slots[i].state=ADP_LIVE;p->block->slots[i].value=value;
    *out=(struct adp_handle){p->identity,i,p->block->slots[i].generation};return done(p,ADP_OK);
  }
  return done(p,ADP_FULL);
}
enum adp_status adp_get(struct adp_pool *p,struct adp_handle h,uint32_t *out) {
  if(!out)return ADP_INVALID;
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(ADP_MUTATION==7)*out=0;
  if(!p->block)return done(p,ADP_CLOSED);
  if(!valid(p,h))return done(p,ADP_STALE);
  *out=p->block->slots[h.index].value;return done(p,ADP_OK);
}
enum adp_status adp_release(struct adp_pool *p,struct adp_handle h) {
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  if(!valid(p,h))return done(p,ADP_STALE);
  struct adp_slot *slot=&p->block->slots[h.index];slot->value=0;
  if(slot->generation==UINT64_MAX) {
    if(ADP_MUTATION==3){slot->generation=1;slot->state=ADP_FREE;}
    else slot->state=ADP_RETIRED;
  } else {++slot->generation;if(ADP_MUTATION==4&&slot->generation<UINT64_MAX)++slot->generation;slot->state=ADP_FREE;}
  return done(p,ADP_OK);
}
enum adp_status adp_snapshot(struct adp_pool *p,struct adp_snapshot *out) {
  if(!out)return ADP_INVALID;
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  struct adp_snapshot next;next.identity=p->identity;
  for(unsigned i=0;i<4;++i)next.slots[i]=p->block->slots[i];
  *out=next;return done(p,ADP_OK);
}
enum adp_status adp_compact(struct adp_pool *p) {
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(!p->block)return done(p,ADP_CLOSED);
  struct adp_block *next=p->ops.allocate(p->ops.context,sizeof *next);
  if(!next)return done(p,ADP_NOMEM);
  for(unsigned i=0;i<4;++i){next->slots[i]=p->block->slots[i];if(ADP_MUTATION==6)next->slots[i].value=0;}
  struct adp_block *old=p->block;p->block=next;p->ops.deallocate(p->ops.context,old);
  return done(p,ADP_OK);
}
enum adp_status adp_move(struct adp_pool *destination,struct adp_pool *source) {
  if(!destination||!source||destination==source)return ADP_INVALID;
  /* Both controls are exclusively accessible here; never copy the mutex. */
  if(!source->block)return ADP_CLOSED;
  if(pthread_mutex_init(&destination->mutex,NULL)!=0)return ADP_SYNC;
  destination->identity=source->identity;
  if(ADP_MUTATION==5)destination->identity.value^=UINT64_C(1);
  destination->block=source->block;destination->ops=source->ops;
  source->block=NULL;return ADP_OK;
}
enum adp_status adp_close(struct adp_pool *p) {
  enum adp_status s=lock(p);if(s!=ADP_OK)return s;
  if(p->block){struct adp_block *old=p->block;p->block=NULL;if(ADP_MUTATION!=8)p->ops.deallocate(p->ops.context,old);}
  return done(p,ADP_OK);
}
enum adp_status adp_destroy(struct adp_pool *p) {
  if(!p)return ADP_INVALID;
  if(p->block)return ADP_STATE;
  return pthread_mutex_destroy(&p->mutex)==0?ADP_OK:ADP_SYNC;
}
