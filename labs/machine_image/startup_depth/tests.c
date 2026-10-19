/* Hosted visible contract tests; libc here is an oracle/transport dependency. */
#include "startup.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STARTUP-CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static void put(unsigned char *p,uint64_t value,unsigned width){for(unsigned i=0;i<width;i++)p[i]=(unsigned char)(value>>(8*i));}
static int stacks(void){
 uint64_t words[]={2,8192,8200,0,12288,0,6,4096,9,4198400,0,0};unsigned char data[97]={0};
 for(unsigned width=4;width<=8;width+=4){
  for(unsigned i=0;i<12;i++)put(data+1+i*width,words[i],width);
  struct sd_stack view;CHECK(sd_stack_decode(data+1,12*width,width,&view)==SD_OK);
  CHECK(view.argc==2&&view.argv==width&&view.envp==4*width&&view.auxv==6*width&&view.extent==12*width&&view.width==width);
  for(size_t n=0;n<12*width;n++){struct sd_stack output;unsigned char saved[sizeof output];memset(&output,0xa5,sizeof output);memcpy(saved,&output,sizeof output);CHECK(sd_stack_decode(data+1,n,width,&output)!=SD_OK);CHECK(memcmp(saved,&output,sizeof output)==0);}
 }
 const unsigned char a[]="first",b[]="second",env[]="K=V";struct sd_mapping maps[]={{12288,env,sizeof env},{8200,b,sizeof b},{8192,a,sizeof a}};const unsigned char *out=NULL;size_t n=777;
 CHECK(sd_string_lookup(8200,maps,3,&out,&n)==SD_OK&&n==6&&!memcmp(out,"second",6));
 CHECK(sd_string_lookup(8193,maps,3,&out,&n)==SD_OK&&n==4&&!memcmp(out,"irst",4));
 n=777;CHECK(sd_string_lookup(8199,maps,3,&out,&n)==SD_RANGE&&n==777);
 maps[2]=maps[1];CHECK(sd_string_lookup(8200,maps,3,&out,&n)==SD_FORMAT);
 maps[2].label=99;maps[2].bytes=(const unsigned char*)"x";maps[2].length=1;CHECK(sd_string_lookup(99,maps,3,&out,&n)==SD_FORMAT);
 unsigned char empty[20]={0};struct sd_stack view;CHECK(sd_stack_decode(empty,20,4,&view)==SD_OK&&view.argc==0&&view.envp==8&&view.auxv==12&&view.extent==20);
 return 0;
}
static int decimals(void){
 uint32_t edge[]={0,1,9,10,99,100,999999999,1000000000,2147483647,2147483648u,UINT32_MAX};
 for(uint32_t i=0;i<65536u+sizeof edge/sizeof edge[0];i++){
  uint32_t value=i<65536?i:edge[i-65536],parsed=123;char expected[16];int digits=snprintf(expected,sizeof expected,"%u",value);CHECK(digits>0&&digits<=10);
  CHECK(sd_parse_u32((const unsigned char*)expected,(size_t)digits,&parsed)==SD_OK&&parsed==value);
  for(int terminated=0;terminated<2;terminated++)for(unsigned mode=0;mode<2;mode++){
   unsigned char output[16];memset(output,0xa5,sizeof output);size_t n=777;enum sd_status status=mode?sd_format_u32_nodiv(value,output,(size_t)digits+(size_t)terminated,terminated,&n):sd_format_u32(value,output,(size_t)digits+(size_t)terminated,terminated,&n);
   CHECK(status==SD_OK&&n==(size_t)digits&&!memcmp(output,expected,n+(size_t)terminated)&&output[n+(size_t)terminated]==0xa5);
   memset(output,0xa5,sizeof output);n=777;status=mode?sd_format_u32_nodiv(value,output,(size_t)digits+(size_t)terminated-1,terminated,&n):sd_format_u32(value,output,(size_t)digits+(size_t)terminated-1,terminated,&n);
   CHECK(status==SD_RANGE&&n==777);for(size_t j=0;j<sizeof output;j++)CHECK(output[j]==0xa5);
  }
 }
 const char *bad[]={"","+1","-1"," 1","1 ","0x1","1_0","4294967296","9999999999","00000000000","\xc2\xb2"};
 for(size_t i=0;i<sizeof bad/sizeof bad[0];i++){uint32_t result=123;CHECK(sd_parse_u32((const unsigned char*)bad[i],strlen(bad[i]),&result)!=SD_OK&&result==123);}
 int32_t signed_edges[]={INT32_MIN,INT32_MIN+1,-1,0,1,INT32_MAX};
 for(size_t i=0;i<sizeof signed_edges/sizeof signed_edges[0];i++){char expected[16];int digits=snprintf(expected,sizeof expected,"%d",signed_edges[i]);unsigned char output[16];size_t n=777;int32_t parsed=99;
  CHECK(sd_parse_i32((const unsigned char*)expected,(size_t)digits,&parsed)==SD_OK&&parsed==signed_edges[i]);CHECK(sd_format_i32(parsed,output,sizeof output,1,&n)==SD_OK&&n==(size_t)digits&&!strcmp((char*)output,expected));}
 for(unsigned i=0;i<2;i++){int32_t parsed=99;const char *s=i?"2147483648":"-2147483649";CHECK(sd_parse_i32((const unsigned char*)s,strlen(s),&parsed)==SD_RANGE&&parsed==99);}
 return 0;
}
struct script {const long *steps;size_t count,at,calls,offsets[16],accepted;unsigned char *base,receiver[64];};
static struct sd_result scripted(void *context,unsigned char *p,size_t n){struct script*s=context;long raw=s->at<s->count?s->steps[s->at++]:-9;s->offsets[s->calls++]=(size_t)(p-s->base);if(raw>0&&(unsigned long)raw<=n){memcpy(s->receiver+s->accepted,p,(size_t)raw);s->accepted+=(size_t)raw;}return sd_linux_result((uint64_t)raw);}
static int transfers(void){
 unsigned char bytes[]="ABCDEFG";struct sd_transfer state;
 const long complete[]={2,-4,3,2};struct script s={.steps=complete,.count=4,.base=bytes};
 CHECK(sd_transfer_init(&state,bytes,7,0)==SD_OK);CHECK(sd_transfer_step(&state,scripted,&s,4,0)==SD_DONE&&state.offset==7&&s.accepted==7&&!memcmp(s.receiver,bytes,7));
 CHECK(s.calls==4&&s.offsets[0]==0&&s.offsets[1]==2&&s.offsets[2]==2&&s.offsets[3]==5);CHECK(sd_transfer_step(&state,scripted,&s,4,0)==SD_DONE&&s.calls==4);
 const long blocked[]={2,-11,5};s=(struct script){.steps=blocked,.count=3,.base=bytes};CHECK(sd_transfer_init(&state,bytes,7,0)==SD_OK);
 CHECK(sd_transfer_step(&state,scripted,&s,5,0)==SD_BLOCKED&&state.offset==2&&s.accepted==2);CHECK(sd_transfer_step(&state,scripted,&s,5,0)==SD_DONE&&state.offset==7&&s.accepted==7&&!memcmp(s.receiver,bytes,7));
 s=(struct script){.steps=blocked,.count=3,.base=bytes};CHECK(sd_transfer_init(&state,bytes,7,0)==SD_OK);CHECK(sd_transfer_step(&state,scripted,&s,1,0)==SD_YIELD&&state.offset==2);CHECK(sd_transfer_step(&state,scripted,&s,1,1)==SD_CANCELLED&&s.calls==1&&s.accepted==2);CHECK(sd_transfer_step(&state,scripted,&s,1,0)==SD_CANCELLED&&s.calls==1);
 const long zero[]={0},partialzero[]={2,0},intr[]={-4,-4,3},excess[]={8};
 for(int reading=0;reading<2;reading++)for(int partial=0;partial<2;partial++){s=(struct script){.steps=partial?partialzero:zero,.count=partial?2:1,.base=bytes};CHECK(sd_transfer_init(&state,bytes,7,reading)==SD_OK);CHECK(sd_transfer_step(&state,scripted,&s,4,0)==(reading?SD_EOF:SD_STALLED)&&state.offset==(size_t)(partial?2:0));}
 s=(struct script){.steps=intr,.count=3,.base=bytes};CHECK(sd_transfer_init(&state,bytes,3,0)==SD_OK);CHECK(sd_transfer_step(&state,scripted,&s,2,0)==SD_YIELD&&state.offset==0&&s.calls==2);CHECK(sd_transfer_step(&state,scripted,&s,1,0)==SD_DONE&&state.offset==3);
 s=(struct script){.steps=excess,.count=1,.base=bytes};CHECK(sd_transfer_init(&state,bytes,7,0)==SD_OK);CHECK(sd_transfer_step(&state,scripted,&s,1,0)==SD_BACKEND_BROKEN&&state.offset==0);
 uint64_t raws[]={0,7,UINT64_MAX-8,UINT64_MAX-3,UINT64_MAX-4094,UINT64_MAX-4095,UINT64_C(0x8000000000000000)};unsigned errors[]={0,0,9,4,4095,0,0};
 for(unsigned i=0;i<7;i++){struct sd_result r=sd_linux_result(raws[i]);CHECK(r.error==errors[i]&&r.failed==(errors[i]!=0)&&r.value==(errors[i]?0:raws[i]));}
 return 0;
}
int main(void){if(stacks()||decimals()||transfers())return 1;puts("STARTUP-CORE-PASS: stack bounds, decimal oracle, partial progress and raw-result range");return 0;}
