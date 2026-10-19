#include "can.h"
#ifndef CN_MUTANT
#define CN_MUTANT 0
#endif
static unsigned field(const uint8_t *p,unsigned n){unsigned v=0;while(n--)v=(v<<1)|*p++;return v;}
static uint16_t crc(const uint8_t *p,unsigned n){
 uint16_t r=0;while(n--){unsigned f=((unsigned)r>>14)^*p++;r=(uint16_t)((r<<1)&0x7fffu);if(f&1)r^=0x4599u;}return r;
}
int cn_feed(CnStream *s,const uint8_t *p,size_t n,size_t *consumed,CnDecoded *out){
 if(!s||(!p&&n)||n>160||!consumed||!out||s->stage>2||s->used>98||s->total>160||s->tail>10||s->run>5||s->expect>1)return CN_INVALID;
 if(s->stage)return CN_BUSY;
 for(size_t i=0;i<n;++i)if(p[i]>1)return CN_INVALID;
 if(CN_MUTANT==1){s->run=0;s->expect=0;}
 *consumed=0;
 for(size_t i=0;i<n;++i){
  unsigned b=p[i];*consumed=i+1;
  if(s->total==160){s->stage=2;return CN_ERROR;}++s->total;
  if(s->needed&&s->used==s->needed&&!s->expect){
   if(s->tail==1)s->ack=b;
   else if(b!=1){s->stage=2;return CN_ERROR;}
   if(++s->tail==10){
    CnDecoded decoded={0};unsigned len=field(s->raw+15,4),body=19+8*len;
    if(CN_MUTANT!=2&&crc(s->raw,body)!=field(s->raw+body,15)){s->stage=2;return CN_ERROR;}
    decoded.frame.identifier=field(s->raw+1,11);decoded.frame.length=(uint8_t)len;decoded.ack=s->ack;
    for(unsigned j=0;j<len;++j)decoded.frame.data[j]=(uint8_t)field(s->raw+19+8*j,8);
    *out=decoded;s->stage=1;return CN_FRAME;
   }
   continue;
  }
  if(s->expect){
   if(b==s->last){s->stage=2;return CN_ERROR;}
   s->last=b;s->run=1;s->expect=0;continue;
  }
  if(s->used==98){s->stage=2;return CN_ERROR;}
  if(s->used==0&&b!=0){s->stage=2;return CN_ERROR;}
  s->raw[s->used++]=(uint8_t)b;s->run=s->run&&b==s->last?s->run+1:1;s->last=b;s->expect=s->run==5;
  if(s->used==19){
   unsigned len=field(s->raw+15,4);
   if(s->raw[12]||s->raw[13]||s->raw[14]||len>8){s->stage=2;return CN_ERROR;}
   s->needed=19+8*len+15;
  }
 }
 return CN_MORE;
}
int cn_search(uint32_t clock,uint32_t rate,unsigned num,unsigned den,uint32_t propagation,CnSearch *out){
 if(!out||!clock||clock>180000000u||!rate||rate>1000000u||!num||num>=den||den>1000||propagation>1000000)return CN_INVALID;
 CnSearch result={0};unsigned best_error=0,best_total=1;
 for(unsigned pre=1;pre<=1024;++pre){
  uint32_t divisor=pre*rate;if(clock%divisor)continue;unsigned total=clock/divisor;
  if(total<3||total>25)continue;
  for(unsigned bs1=1;bs1<=16;++bs1){
   if(total<=1+bs1)continue;
   unsigned bs2=total-1-bs1;if(bs2>8)continue;
   if(CN_MUTANT!=3&&(uint64_t)bs1*pre*1000000000u<(uint64_t)propagation*clock)continue;
   unsigned a=(1+bs1)*den,b=num*total,error=a>b?a-b:b-a;
   ++result.candidates;
   if(result.candidates==1||error*best_total<best_error*total){
    if(sd_can_timing(clock,pre,bs1,bs2,1,&result.timing)!=SD_OK)return CN_ERROR;
    best_error=error;best_total=total;result.sample_num=1+bs1;result.sample_den=total;
   }
  }
 }
 if(!result.candidates)return CN_UNAVAILABLE;
 *out=result;return CN_OK;
}
