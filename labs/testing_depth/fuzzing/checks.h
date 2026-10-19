#ifndef TF_CHECKS_H
#define TF_CHECKS_H
#include "fuzzing.h"
#include <string.h>
/* A separate mathematical byte fold; no calls to either production decoder. */
static unsigned expected(unsigned profile,const unsigned char *p,size_t n,TfPacket *out){
 TfPacket staged={0};size_t header=profile?3:2;
 if(n>(profile?12u:10u)||n<header)return TF_LENGTH;
 if(p[0]!=(profile?2:1))return TF_OPCODE;
 if(p[1]>4)return TF_COUNT;
 if(profile&&p[2]>1)return TF_FLAGS;
 unsigned flag=profile?p[2]:0;
 if(n!=header+2*(size_t)p[1]+flag)return TF_LENGTH;
 staged.packet.count=p[1];
 for(size_t i=header;i<n-flag;i++)staged.packet.values[(i-header)/2]=(uint16_t)(staged.packet.values[(i-header)/2]*256u+p[i]);
 staged.has_tag=flag;staged.tag=flag?p[n-1]:0;*out=staged;return TF_OK;
}
static int one_case(const unsigned char *p,size_t n){
 for(unsigned profile=0;profile<2;profile++){
  TfPacket out,repeat,oracle;memset(&out,0x5a,sizeof out);repeat=out;oracle=out;
  unsigned wanted=expected(profile,p,n,&oracle),a=tf_parse(profile,p,n,&out),b=tf_parse(profile,p,n,&repeat);
  if(a!=wanted||b!=wanted)return 0;
  if(wanted!=TF_OK){if(memcmp(&out,&oracle,sizeof out)||memcmp(&repeat,&oracle,sizeof repeat))return 0;}
  else{
   if(out.packet.count!=oracle.packet.count||out.has_tag!=oracle.has_tag||out.tag!=oracle.tag)return 0;
   for(unsigned i=0;i<4;i++)if(out.packet.values[i]!=oracle.packet.values[i]||repeat.packet.values[i]!=oracle.packet.values[i])return 0;
  }
 }
 return 1;
}
#endif
