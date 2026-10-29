#include "fixed_point.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_q7_even(const int8_t*a,const int8_t*b,size_t n,int8_t*out){
 if(!out||((!a||!b)&&n))return B2_INVALID;
 if(n>1024)return B2_RANGE;
 int32_t sum=0;for(size_t i=0;i<n;++i)sum+=(int32_t)a[i]*(int32_t)b[i];
 uint32_t magnitude=(uint32_t)(sum<0?-sum:sum),q=magnitude/128u,r=magnitude%128u;
 if(r>64u||(r==64u&&((q&1u)||BF_MUTANT==2)))++q;
 int32_t rounded=sum<0?-(int32_t)q:(int32_t)q;if(rounded>127)rounded=127;if(rounded< -128)rounded=-128;*out=(int8_t)rounded;return B2_OK;
}
int bf_q11(const int16_t*a,const int16_t*b,size_t n,int16_t*out){
 if(!out||((!a||!b)&&n))return B2_INVALID;
 if(n>65536)return B2_RANGE;
 int64_t sum=0;for(size_t i=0;i<n;++i){sum+=(int64_t)((int32_t)a[i]*(int32_t)b[i]);if(BF_MUTANT==3){if(sum>INT32_MAX)sum=INT32_MAX;if(sum<INT32_MIN)sum=INT32_MIN;}}
 uint64_t magnitude=(uint64_t)(sum<0?-sum:sum);uint64_t q=BF_MUTANT==4?(magnitude+64u)>>7:(magnitude+1024u)>>11;
 int64_t rounded=sum<0?-(int64_t)q:(int64_t)q;if(rounded>32767)rounded=32767;if(rounded< -32768)rounded=-32768;*out=(int16_t)rounded;return B2_OK;
}
int bf_signal(const int8_t*samples,const int8_t*gains,size_t n,struct bf_signal*out){
 if(!out)return B2_INVALID;
 int8_t value;int rc=b2_q7(samples,gains,n,&value);if(rc!=B2_OK)return rc;
 int32_t sum=0;for(size_t i=0;i<n;++i)sum+=(int32_t)samples[i]*(int32_t)gains[i];
 int32_t rounded=sum<0?-((-sum+64)/128):(sum+64)/128;
 struct bf_signal result={sum,rounded,(int32_t)value*128-sum,value,rounded>127||rounded< -128};*out=result;return B2_OK;
}
static struct bf_small small(int32_t numerator,int32_t denominator,unsigned mode){
 uint32_t magnitude=(uint32_t)(numerator<0?-numerator:numerator),d=(uint32_t)denominator,q=magnitude/d,r=magnitude%d;
 if(r>d-r||(r==d-r&&(mode==BF_AWAY||(q&1u)||BF_MUTANT==5)))++q;
 int32_t rounded=numerator<0?-(int32_t)q:(int32_t)q,output=rounded;
 if(output>127)output=127;
 if(output< -128)output=-128;
 struct bf_small result={numerator,denominator,rounded,output*denominator-numerator,(int8_t)output,output!=rounded};return result;
}
int bf_mul16(int8_t a,int8_t b,unsigned mode,struct bf_small*out){
 if(!out||mode>BF_EVEN)return B2_INVALID;
 *out=small((int32_t)a*(int32_t)b,16,mode);return B2_OK;
}
int bf_mix16_32(int8_t a,int8_t b,unsigned mode,struct bf_small*out){
 if(!out||mode>BF_EVEN)return B2_INVALID;
 *out=small((BF_MUTANT==6?1:2)*(int32_t)a+(int32_t)b,2,mode);return B2_OK;
}
int bf_two16(int8_t a,int8_t b,int8_t c,unsigned mode,struct bf_cascade*out){
 if(!out||mode>BF_EVEN)return B2_INVALID;
 struct bf_cascade result;result.first=small((int32_t)a*(int32_t)b,16,mode);result.second=small((int32_t)result.first.output*(int32_t)c,16,mode);
 result.error_numerator=(int32_t)result.second.output*256-(int32_t)a*(int32_t)b*(int32_t)c;
 result.bound_numerator=BF_MUTANT==7?128u:8u*(unsigned)(c<0?-(int32_t)c:c)+128u;result.bounded=!result.first.clipped&&!result.second.clipped;*out=result;return B2_OK;
}
