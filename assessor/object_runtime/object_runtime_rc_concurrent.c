#include "object_runtime_rc_concurrent.h"
#include <stdlib.h>
static void unlock(struct orr_concurrent *h) { if(pthread_mutex_unlock(&h->mutex))abort(); }
enum or_status orrc_init(struct orr_concurrent *h) {
  if(!h)return OR_INVALID;
  if(pthread_mutex_init(&h->mutex,NULL))return OR_FULL;
  orr_init(&h->heap);h->initialized=1;return OR_OK;
}
enum or_status orrc_call(struct orr_concurrent *h,const struct orr_request *q,struct orr_response *out) {
  struct orr_response r={0};enum or_status s;
  if(!h || !q || !out || !h->initialized)return OR_INVALID;
  if(pthread_mutex_lock(&h->mutex))return OR_STATE;
  switch(q->operation) {
  case ORR_NEW:s=orr_new(&h->heap,&r.object);break;
  case ORR_RETAIN:s=orr_retain(&h->heap,q->object);break;
  case ORR_RELEASE:s=orr_release(&h->heap,q->object,q->defer,&r.dead);break;
  case ORR_STORE:s=orr_store(&h->heap,q->object,q->field,q->target,q->defer,&r.dead);break;
  case ORR_FLUSH:s=orr_flush(&h->heap,q->budget,&r.processed,&r.dead);break;
  case ORR_PREPARE:s=orr_prepare(&h->heap,&r.trial);break;
  case ORR_COMMIT:s=orr_commit(&h->heap,&q->trial,&r.dead);break;
  case ORR_INSPECT:r.invariant=(unsigned)orr_invariant(&h->heap);r.live=h->heap.counts.live;r.revision=h->heap.revision;s=OR_OK;break;
  default:s=OR_INVALID;break;
  }
  if(s==OR_OK || s==OR_DONE)*out=r;
  unlock(h);return s;
}
enum or_status orrc_close(struct orr_concurrent *h) {
  enum or_status s;
  if(!h || !h->initialized)return OR_INVALID;
  if(pthread_mutex_lock(&h->mutex))return OR_STATE;
  s=orr_close(&h->heap);unlock(h);if(s!=OR_OK)return s;
  if(pthread_mutex_destroy(&h->mutex))abort();
  h->initialized=0;return OR_OK;
}
