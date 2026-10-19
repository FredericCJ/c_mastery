#include "systems.h"
int s_checksum_add(SChecksum *s,const unsigned char *p,size_t n){
 if(!s||(!p&&n)||s->sum>65535||s->pending>1||s->octet>255)return S_INVALID;
 for(size_t i=0;i<n;i++){
  if(!s->pending){s->octet=p[i];s->pending=1;}
  else {s->sum+=(s->octet<<8)|p[i];s->sum=(s->sum&65535)+(s->sum>>16);s->pending=0;}
 }
#ifdef S_MUTATE_CHECKSUM
 if(s->pending){s->sum+=s->octet<<8;s->sum=(s->sum&65535)+(s->sum>>16);s->pending=0;}
#endif
 return S_OK;
}
int s_checksum_finish(const SChecksum *s,uint16_t *out){
 if(!s||!out||s->sum>65535||s->pending>1||s->octet>255)return S_INVALID;
 uint32_t sum=s->sum+(s->pending?s->octet<<8:0);sum=(sum&65535)+(sum>>16);
 *out=(uint16_t)(sum^65535);return S_OK;
}
