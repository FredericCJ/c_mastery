#include "models.h"
#ifndef TM_MUTANT
#define TM_MUTANT 0
#endif
void tm_init(TmQueue *q){(void)ta_init(&q->core,&q->storage,TA_RING,2,3);q->revision=1;}
TaStatus tm_push(TmQueue *q,unsigned char v){
 if(q->revision==UINT64_MAX)return TA_INVALID;
 TaStatus status=ta_push(&q->core,v);
 if(status==TA_OK)q->revision++;
 else if(TM_MUTANT==1)q->storage.bytes[TA_MAX]^=1;
 return status;
}
TaStatus tm_pop(TmQueue *q,unsigned char *v){
 if(q->revision==UINT64_MAX)return TA_INVALID;
 TaStatus status=ta_pop(&q->core,v);
 if(status==TA_OK&&TM_MUTANT!=2)q->revision++;
 return status;
}
TaStatus tm_clear(TmQueue *q){
 unsigned char v;
 if(!ta_count(&q->core))return TA_OK;
 if(q->revision==UINT64_MAX)return TA_INVALID;
 if(TM_MUTANT!=3)while(ta_pop(&q->core,&v)==TA_OK){}
 q->revision++;return TA_OK;
}
TaStatus tm_borrow(const TmQueue *q,TmBorrow *out){
 if(!out)return TA_INVALID;
 if(!ta_count(&q->core))return TA_EMPTY;
 *out=(TmBorrow){q,&q->storage.bytes[q->core.head],q->revision};return TA_OK;
}
TaStatus tm_read(TmBorrow b,unsigned char *out){
 if(!out||!b.owner||!b.revision)return TA_INVALID;
 if(b.revision!=b.owner->revision||!ta_count(&b.owner->core)||b.pointer!=&b.owner->storage.bytes[b.owner->core.head])return TA_INVALID;
 *out=*b.pointer;return TA_OK;
}
unsigned tm_decode(const unsigned char *p,size_t n,TmPacket *out){
 TmPacket staged={0};size_t expected;
 if(!out||(!p&&n))return TM_INVALID;
 if(n<2)return TM_SHORT;
 if(p[0]!=1)return TM_OPCODE;
 if(p[1]>4)return TM_COUNT;
 expected=2+2*(size_t)p[1];
 if(TM_MUTANT==5)expected-=2;
 if(n<expected)return TM_SHORT;
 if(n>expected)return TM_TRAILING;
 staged.count=p[1];
 for(unsigned i=0;i<staged.count;i++){
  if(2+2*(size_t)i+1>=n)return TM_SHORT;
  unsigned a=p[2+2*i],b=p[3+2*i];staged.values[i]=(uint16_t)(TM_MUTANT==4?(b*256+a):(a*256+b));
 }
 *out=staged;return TM_OK;
}
unsigned tm_reference(const unsigned char *p,size_t n,int permissive,TmPacket *out){
 TmPacket staged={0};unsigned tag=0,count=0,high=0,complete=0;
 if(!out||(!p&&n)||(permissive!=0&&permissive!=1))return TM_INVALID;
 /* Different representation: a byte-state fold with separate error arbitration. */
 for(size_t i=0;i<n;i++){
  if(i==0)tag=p[i];
  else if(i==1)count=permissive?(p[i]&127u):p[i];
  else if((i&1u)==0)high=p[i];
  else {if(complete<4)staged.values[complete]=(uint16_t)(high*256+p[i]);complete++;}
 }
 if(n<2)return TM_SHORT;
 if(tag!=1)return TM_OPCODE;
 if(count>4)return TM_COUNT;
 if(n<2+2*(size_t)count)return TM_SHORT;
 if(n>2+2*(size_t)count)return TM_TRAILING;
 staged.count=count;*out=staged;return TM_OK;
}
unsigned tm_fir(TmFir *f,const int32_t *p,size_t n,int saturated,int64_t *out){
 if(!f||(!p&&n)||(!out&&n)||(saturated!=0&&saturated!=1)||n>64)return TM_INVALID;
 for(size_t i=0;i<n;i++)if(p[i]<-1000000||p[i]>1000000)return TM_INVALID;
 if(f->previous<-1000000||f->previous>1000000)return TM_INVALID;
 int32_t previous=TM_MUTANT==6?0:f->previous;
 for(size_t i=0;i<n;i++){
  int64_t numerator=(int64_t)p[i]+previous;
  if(saturated){if(numerator>4)numerator=4;if(numerator< -4)numerator=-4;}
  out[i]=numerator;previous=p[i];
 }
 f->previous=previous;return TM_OK;
}
