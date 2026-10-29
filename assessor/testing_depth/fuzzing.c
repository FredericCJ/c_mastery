#include "fuzzing.h"
#ifndef TF_MUTANT
#define TF_MUTANT 0
#endif
unsigned tf_parse(unsigned profile,const unsigned char *p,size_t n,struct tf_packet *out){
 struct tf_packet staged={0};unsigned char canonical[10];unsigned status;
 if(!out||profile>1)return TF_INVALID;
 if(n>(profile?12u:10u))return TF_LENGTH;
 if(!p&&n)return TF_INVALID;
 if(!profile){
  status=tm_decode(p,n,&staged.packet);
  if(status!=TM_OK){if(TF_MUTANT==1)out->tag=99;return status==TM_OPCODE?TF_OPCODE:status==TM_COUNT?TF_COUNT:TF_LENGTH;}
 }else{
  if(n<3)return TF_LENGTH;
  if(p[0]!=2)return TF_OPCODE;
  if(p[1]>4)return TF_COUNT;
  if(p[2]>1)return TF_FLAGS;
  size_t expected=3+2*(size_t)p[1]+p[2];
  if(n!=expected)return TF_LENGTH;
  canonical[0]=1;canonical[1]=p[1];
  for(size_t i=0;i<2*(size_t)p[1];i++)canonical[2+i]=p[3+i];
  status=tm_decode(canonical,2+2*(size_t)p[1],&staged.packet);
  if(status!=TM_OK)return TF_INVALID;
  staged.has_tag=p[2];staged.tag=p[2]?p[n-1]:0;
  if(TF_MUTANT==2)staged.tag=0;
 }
 if(TF_MUTANT==3&&staged.packet.count)staged.packet.values[0]^=1;
 *out=staged;return TF_OK;
}
void tf_session_init(struct tf_session *s){s->epoch=0;s->next=0;s->total=0;s->active=0;}
unsigned tf_session_begin(struct tf_session *s,uint32_t epoch){
 if(!epoch||epoch<=s->epoch)return TF_SESSION;
 s->epoch=epoch;s->next=0;s->total=0;s->active=1;return TF_OK;
}
unsigned tf_session_apply(struct tf_session *s,uint32_t epoch,uint32_t sequence,const unsigned char *p,size_t n,struct tf_packet *out){
 struct tf_packet staged;uint32_t sum=0;unsigned result;
 if(!out)return TF_INVALID;
 if(!s->active||s->epoch!=epoch)return TF_SESSION;
 if(s->next>UINT16_MAX||sequence!=s->next)return TF_SEQUENCE;
 result=tf_parse(0,p,n,&staged);if(result!=TF_OK)return result;
 for(unsigned i=0;i<staged.packet.count;i++)sum+=staged.packet.values[i];
 if(sum>UINT32_MAX-s->total)return TF_RANGE;
 s->total+=sum;if(TF_MUTANT!=4)s->next++;
 *out=staged;return TF_OK;
}
