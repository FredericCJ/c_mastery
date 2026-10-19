#include "architecture.h"
#ifndef TA_MUTANT
#define TA_MUTANT 0
#endif
TaStatus ta_init(TaQueue *q,TaStorage *s,TaLayout layout,unsigned cap,unsigned extent){
 unsigned need;
 if(!q||!s||layout<TA_RING||layout>TA_COUNTED||cap==0||cap>TA_MAX)return TA_INVALID;
 need=cap+(layout==TA_RING);
 if(extent!=need)return TA_INVALID;
 q->storage=s;q->layout=layout;q->capacity=cap;q->extent=extent;
 q->count=0;q->head=layout==TA_LINKED?TA_NONE:0;
 q->tail=q->head;q->free_head=0;
 if(layout==TA_LINKED)for(unsigned i=0;i<cap;i++)s->nodes[i].next=i+1<cap?i+1:TA_NONE;
 return TA_OK;
}
size_t ta_count(const TaQueue *q){return q->count;}
TaStatus ta_push(TaQueue *q,unsigned char value){
 if(q->count==q->capacity){
  if(TA_MUTANT==1)q->storage->bytes[0]=value;
  return TA_FULL;
 }
 if(q->layout==TA_LINKED){
  unsigned at=q->free_head;q->free_head=q->storage->nodes[at].next;
  q->storage->nodes[at].value=value;q->storage->nodes[at].next=TA_NONE;
  if(q->count)q->storage->nodes[q->tail].next=at;else q->head=at;
  q->tail=at;
 }else if(q->layout==TA_COMPACT)q->storage->bytes[q->count]=value;
 else {q->storage->bytes[q->tail]=value;q->tail=(q->tail+1)%q->extent;}
 q->count++;return TA_OK;
}
TaStatus ta_peek(const TaQueue *q,unsigned char *out){
 if(!out)return TA_INVALID;
 if(!q->count){if(TA_MUTANT==2)*out=0;return TA_EMPTY;}
 *out=q->layout==TA_LINKED?q->storage->nodes[q->head].value:q->storage->bytes[q->layout==TA_COMPACT?0:q->head];
 return TA_OK;
}
TaStatus ta_pop(TaQueue *q,unsigned char *out){
 TaStatus s=ta_peek(q,out);if(s!=TA_OK)return s;
 if(q->layout==TA_LINKED){
  unsigned old=q->head;q->head=q->storage->nodes[old].next;
  q->storage->nodes[old].next=q->free_head;q->free_head=old;
  if(q->count==1)q->tail=TA_NONE;
 }else if(q->layout==TA_COMPACT){
  for(unsigned i=1;i<q->count;i++)q->storage->bytes[i-1]=q->storage->bytes[i];
 }else q->head=(q->head+1)%q->extent;
 if(TA_MUTANT!=3)q->count--;
 return TA_OK;
}
TaStatus ta_batch(TaQueue *q,const unsigned char *p,size_t n,size_t *accepted){
 if(!accepted)return TA_INVALID;
 *accepted=0;if(n&&!p)return TA_INVALID;
 while(*accepted<n){TaStatus s=ta_push(q,p[*accepted]);if(s!=TA_OK)return s;(*accepted)++;}
 return TA_OK;
}
int ta_invariant(const TaQueue *q){
 if(!q||!q->storage||q->layout<TA_RING||q->layout>TA_COUNTED||!q->capacity||q->capacity>TA_MAX||q->count>q->capacity)return 0;
 if(q->extent!=q->capacity+(q->layout==TA_RING))return 0;
 if(q->layout==TA_LINKED){
  unsigned mask=0,n=0,at=q->head,last=TA_NONE;
  while(at!=TA_NONE){if(at>=q->capacity||(mask&(1u<<at)))return 0;mask|=1u<<at;n++;last=at;at=q->storage->nodes[at].next;}
  if(n!=q->count||last!=q->tail)return 0;
  at=q->free_head;
  while(at!=TA_NONE){if(at>=q->capacity||(mask&(1u<<at)))return 0;mask|=1u<<at;at=q->storage->nodes[at].next;}
  return mask==(1u<<q->capacity)-1;
 }
 if(q->layout==TA_COMPACT)return q->head==0&&q->tail==0;
 return q->head<q->extent&&q->tail<q->extent&&q->tail==(q->head+q->count)%q->extent;
}
TaStatus ta_decode(const unsigned char *p,size_t n,unsigned char out[2]){
 if(!p||!out||n!=3||p[0]!=2)return TA_INVALID;
 out[0]=p[1];out[1]=p[2];return TA_OK;
}
TaStatus ta_begin(TaFlow *f,const unsigned char *p,size_t n,TaLayout layout,unsigned budget){
 unsigned char decoded[2];size_t accepted;
 if(!f||!budget||layout<TA_RING||layout>TA_COUNTED||ta_decode(p,n,decoded)!=TA_OK)return TA_INVALID;
 (void)ta_init(&f->queue,&f->storage,layout,2,2+(layout==TA_RING));
 (void)ta_batch(&f->queue,decoded,2,&accepted);
 (void)ta_pop(&f->queue,&f->bytes[0]);(void)ta_pop(&f->queue,&f->bytes[1]);
 f->offset=0;f->budget=budget;f->state=TA_AGAIN;f->uncertain=0;return TA_OK;
}
TaStatus ta_step(TaFlow *f,TaWrite write,void *context){
 TaWriteResult r;size_t remaining;
 if(f->state!=TA_AGAIN)return f->state;
 if(!write)return TA_INVALID;
 remaining=2-f->offset;r=write(context,f->bytes+f->offset,remaining);
 f->budget--;
 if(r.status<TA_WRITE_OK||r.status>TA_WRITE_ERROR||r.count>remaining||(r.status==TA_WRITE_WAIT&&r.count)){
  f->uncertain=1;return f->state=TA_INVALID;
 }
 if(TA_MUTANT!=4)f->offset+=r.count;
 if(r.status==TA_WRITE_ERROR)return f->state=TA_IO;
 if(f->offset==2)return f->state=TA_DONE;
 if(!f->budget&&TA_MUTANT!=5)return f->state=TA_TIMEOUT;
 return TA_AGAIN;
}
TaStatus ta_cancel(TaFlow *f){
 if(f->state==TA_AGAIN)f->state=TA_CANCELLED;
 return f->state;
}
