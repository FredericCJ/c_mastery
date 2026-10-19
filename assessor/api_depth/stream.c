#include "stream.h"
#ifndef SF_MUTANT
#define SF_MUTANT 0
#endif
void sf_init(SFStream*s){if(s)*s=(SFStream){0};}
int sf_start(SFStream*s,const unsigned char*bytes,size_t n,SFHandle*out){
 if(!s||!out||(!bytes&&n))return SF_INVALID;if(n>64)return SF_RANGE;if(s->state==1)return SF_STATE;if(s->generation==UINT32_MAX)return SF_RANGE;
 SFStream next={.length=n+2,.generation=s->generation+1,.state=1};next.bytes[0]=(unsigned char)n;unsigned sum=0;
 for(size_t i=0;i<n;i++){next.bytes[i+1]=bytes[i];sum=(sum+bytes[i])&255u;}next.bytes[n+1]=(unsigned char)sum;*s=next;*out=(SFHandle){s,s->generation};return SF_OK;
}
static int valid(SFHandle h){return h.owner&&h.generation&&h.generation==h.owner->generation;}
int sf_pump(SFHandle h,SFWrite write,void*ctx,unsigned budget,size_t*accepted){
 if(!valid(h)||!write||!accepted)return SF_INVALID;SFStream*s=h.owner;if(s->offset>s->length||s->length>66)return SF_INVALID;*accepted=s->offset;
 if(s->state==2)return SF_DONE;if(s->state==3)return SF_CANCELLED;if(s->state!=1)return SF_STATE;
 while(budget--){size_t remaining=s->length-s->offset;ptrdiff_t n=write(ctx,s->bytes+s->offset,remaining);
  if(n==-2)continue;if(n==0)return SF_PENDING;if(n<0){s->state=4;return SF_IO;}if((size_t)n>remaining){s->state=4;return SF_INVALID;}
  if(SF_MUTANT!=1)s->offset+=(size_t)n;*accepted=s->offset;if(s->offset==s->length){s->state=2;return SF_DONE;}
 }return SF_PENDING;
}
int sf_cancel(SFHandle h){if(!valid(h))return SF_INVALID;if(h.owner->state!=1)return SF_STATE;if(SF_MUTANT!=2)h.owner->state=3;return SF_OK;}
