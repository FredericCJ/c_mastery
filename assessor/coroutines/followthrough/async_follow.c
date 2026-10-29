#include "async_follow.h"
#ifndef CA_MUTANT
#define CA_MUTANT 0
#endif
int ca_create(struct co_allocator a,unsigned wanted,uint32_t generation,struct ca_frame**out){
 if(!out||!a.allocate||!a.release||!generation)return CO_INVALID;if(!wanted||wanted>8)return CO_RANGE;
 struct ca_frame*f=a.allocate(a.context,sizeof *f);if(!f)return CO_RANGE;*f=(struct ca_frame){.allocator=a,.generation=generation,.next=1,.wanted=wanted};
 if(pthread_mutex_init(&f->mutex,NULL)){a.release(a.context,f);return CO_IO;}*out=f;return CO_OK;
}
int ca_reserve(struct ca_frame*f,struct ca_ticket*out){
 if(!f||!out)return CO_INVALID;pthread_mutex_lock(&f->mutex);int r=CO_RANGE;
 if(f->closing)r=CO_STATE;else if(f->next!=UINT32_MAX)for(unsigned i=0;i<4;i++)if(!f->tickets[i]){f->tickets[i]=f->next++;f->inflight++;*out=(struct ca_ticket){f,f->generation,f->tickets[i],i};r=CO_OK;break;}
 pthread_mutex_unlock(&f->mutex);return r;
}
int ca_notify(struct ca_ticket*t,const unsigned char*data,size_t n){
 if(!t||!t->owner||(!data&&n))return CO_INVALID;struct ca_frame*f=t->owner;pthread_mutex_lock(&f->mutex);int r=CO_STATE;
 if(t->generation==f->generation&&t->slot<4&&t->serial&&f->tickets[t->slot]==t->serial){
  f->tickets[t->slot]=0;f->inflight--;t->owner=NULL;
  if(f->state==0||(CA_MUTANT==2&&f->state==1)){
   if(n>f->wanted-f->length){f->state=3;f->closing=1;r=CO_RANGE;}
   else{for(size_t i=0;i<n;i++)f->data[f->length++]=data[i];r=CO_OK;if(f->length==f->wanted){f->state=2;f->closing=1;}}
  }
 }
 pthread_mutex_unlock(&f->mutex);return r;
}
int ca_cancel(struct ca_frame*f){if(!f)return CO_INVALID;pthread_mutex_lock(&f->mutex);int r=CO_STATE;if(!f->state){f->state=1;f->closing=1;r=CO_OK;}pthread_mutex_unlock(&f->mutex);return r;}
int ca_ack(struct ca_frame*f){if(!f)return CO_INVALID;pthread_mutex_lock(&f->mutex);int r=!f->closing?CO_STATE:f->inflight?CO_PENDING:CO_OK;if(!r)f->ack=1;pthread_mutex_unlock(&f->mutex);return r;}
int ca_poll(struct ca_frame*f,unsigned char*out,size_t*n){
 if(!f||!out||!n)return CO_INVALID;pthread_mutex_lock(&f->mutex);int r=CO_PENDING;
 if(f->ack){r=f->state==2?CO_DONE:f->state==1?CO_STATE:CO_IO;if(r==CO_DONE&&!f->published){for(unsigned i=0;i<f->length;i++)out[i]=f->data[i];*n=f->length;f->published=1;}}
 pthread_mutex_unlock(&f->mutex);return r;
}
int ca_destroy(struct ca_frame**owner){
 if(!owner)return CO_INVALID;if(!*owner)return CO_OK;struct ca_frame*f=*owner;pthread_mutex_lock(&f->mutex);
 if(CA_MUTANT!=1&&(!f->ack||f->inflight)){pthread_mutex_unlock(&f->mutex);return CO_STATE;}
 struct co_allocator a=f->allocator;pthread_mutex_unlock(&f->mutex);pthread_mutex_destroy(&f->mutex);*owner=NULL;a.release(a.context,f);return CO_OK;
}
