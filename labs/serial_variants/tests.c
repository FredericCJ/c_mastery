#include "variants.h"
#include <string.h>
#include <limits.h>
void sv_note(const char *);
static void number(char *s,unsigned *n,uint32_t x){char d[10];unsigned k=0;do{d[k++]=(char)('0'+x%10u);x/=10u;}while(x);while(k)s[(*n)++]=d[--k];}
static void values(const char *name,uint32_t a,uint32_t b,uint32_t c,uint32_t d){char s[100];unsigned n=0;const char *p="SV_VALUES ";while(*p)s[n++]=*p++;while(*name)s[n++]=*name++;uint32_t v[4]={a,b,c,d};for(unsigned i=0;i<4;i++){s[n++]=' ';number(s,&n,v[i]);}s[n++]='\n';s[n]=0;sv_note(s);}
#define CHECK(x) do{if(!(x)){sv_note("SV_ASSERT:" #x "\n");return 41;}}while(0)
static int baud(void){
 const uint32_t clocks[]={48000000,80000000,125000000};const uint32_t rates[]={9600,115200,1000000};
 for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++){SvBaud out={91,92,93};CHECK(sv_baud(clocks[i],rates[j],10000,&out)==SV_OK);CHECK(out.integer==out.units/64u&&out.fraction==out.units%64u);values("baud",clocks[i],rates[j],out.integer,out.fraction);}
 SvBaud out={91,92,93};CHECK(sv_baud(80000000,115200,79,&out)==SV_INVALID&&out.integer==91&&out.fraction==92&&out.units==93);
 CHECK(sv_baud(80000000,115200,80,&out)==SV_OK&&out.integer==43&&out.fraction==26);
 out=(SvBaud){91,92,93};CHECK(sv_baud(1000000,100000,1000000,&out)==SV_INVALID&&out.integer==91);
 CHECK(sv_baud(180000000,1,1000000,&out)==SV_INVALID&&out.integer==91);
 CHECK(sv_baud(UINT32_MAX,115200,1000000,&out)==SV_INVALID&&out.integer==91);
 CHECK(sv_baud(0,115200,1000000,&out)==SV_INVALID&&sv_baud(80000000,0,0,&out)==SV_INVALID);
 sv_note("SV_BAUD_PASS\n");return 0;
}
static int word(void){
 unsigned count=0,checksum=0;
 for(unsigned m=0;m<2;m++)for(unsigned par=0;par<3;par++)for(unsigned stop=1;stop<3;stop++)for(unsigned p=0;p<2;p++)for(unsigned h=0;h<2;h++)for(unsigned l=0;l<2;l++)for(unsigned v=0;v<(1u<<(8u+m-(par!=0)));v++){
  SvWord out;memset(&out,0xa5,sizeof out);CHECK(sv_word(m,par,stop,p,h,l,(uint16_t)v,&out)==SV_OK);
  unsigned bits=8u+m,payload=bits-(par!=0),parity=par==2;CHECK(out.length==1u+bits+stop&&out.payload==payload&&out.edges==2u*(bits-!l)&&out.wire[0]==0);
  for(unsigned i=0;i<payload;i++){CHECK(out.wire[i+1]==((v>>i)&1u));parity^=(v>>i)&1u;}
  if(par)CHECK(out.wire[bits]==parity);
  for(unsigned i=bits+1;i<out.length;i++)CHECK(out.wire[i]==1);
  for(unsigned i=0;i<out.edges;i++)CHECK(out.clock[i]==(p^((i+1)&1u))&&out.sample[i]==((i&1u)==h));
  for(unsigned i=0;i<out.length;i++)checksum+=out.wire[i];
  count++;
 }
 SvWord a;memset(&a,0x55,sizeof a);SvWord before=a;CHECK(sv_word(1,1,1,0,0,1,256,&a)==SV_INVALID&&!memcmp(&a,&before,sizeof a));
 CHECK(sv_word(1,0,1,0,0,0,511,&a)==SV_OK&&a.edges==16&&a.wire[9]==1);
 CHECK(sv_word(1,1,1,0,0,0,255,&a)==SV_OK&&a.edges==16&&a.wire[9]==0);
 values("word",count,checksum,a.length,a.edges);sv_note("SV_WORD_PASS\n");return 0;
}
static int capture(void){
 unsigned rejected=0;
 for(unsigned p=0;p<2;p++)for(unsigned h=0;h<2;h++){
  SvEdge e[18];for(unsigned i=0;i<18;i++)e[i]=(SvEdge){p^((i+1)&1u),(i&1u)==h,10,10};SvCapture out={99,99};
  CHECK(sv_capture(p,h,160,e,18,&out)==SV_OK&&out.defects==0);
  CHECK(sv_capture(p^1u,h,160,e,18,&out)==SV_IO&&(out.defects&1));rejected++;
  CHECK(sv_capture(p,h^1u,160,e,18,&out)==SV_IO&&(out.defects&2));rejected++;
  e[h].setup_ns=9;CHECK(sv_capture(p,h,160,e,18,&out)==SV_IO&&out.index==h&&out.defects==4);rejected++;
  e[h].setup_ns=10;e[h].hold_ns=9;CHECK(sv_capture(p,h,160,e,18,&out)==SV_IO&&out.index==h&&out.defects==8);rejected++;
  e[h].hold_ns=11;e[h].setup_ns=11;CHECK(sv_capture(p,h,161,e,18,&out)==SV_IO);/* other samples still10 */
  out=(SvCapture){99,98};e[17].level=2;CHECK(sv_capture(p,h,160,e,18,&out)==SV_INVALID&&out.index==99&&out.defects==98);
 }
 values("capture",rejected,10,11,18);sv_note("SV_CAPTURE_PASS\n");return 0;
}
static int receiver(void){
 unsigned value=99;
 CHECK(sv_receiver(0,0,1,0,0,0,0,SV_UNKNOWN,&value)==SV_OK&&value==SV_HIGH);
 CHECK(sv_receiver(-120,0,1,0,0,0,0,SV_HIGH,&value)==SV_OK&&value==SV_UNKNOWN);
 const int inputs[]={-201,-120,0,-120,-201};const unsigned expected[]={SV_LOW,SV_LOW,SV_HIGH,SV_HIGH,SV_LOW};unsigned prior=SV_UNKNOWN,sum=0;
 for(unsigned i=0;i<5;i++){CHECK(sv_receiver(inputs[i],0,1,0,1,-130,-100,prior,&value)==SV_OK&&value==expected[i]);prior=value;sum+=value;}
 CHECK(sv_receiver(14000,14000,1,0,0,0,0,SV_UNKNOWN,&value)==SV_OK&&value==SV_HIGH);
 value=99;CHECK(sv_receiver(15001,15001,1,0,0,0,0,SV_UNKNOWN,&value)==SV_INVALID&&value==99);
 CHECK(sv_receiver(-15000,15000,1,0,0,0,0,SV_UNKNOWN,&value)==SV_INVALID&&value==99);
 CHECK(sv_receiver(0,0,1,0,1,-110,-100,SV_UNKNOWN,&value)==SV_INVALID&&value==99);
 CHECK(sv_receiver(0,0,0,0,0,0,0,SV_UNKNOWN,&value)==SV_OK&&value==SV_HIGHZ);
 CHECK(sv_receiver(0,0,1,1,0,0,0,SV_UNKNOWN,&value)==SV_OK&&value==SV_HIGH);
 values("receiver",sum,SV_UNKNOWN,SV_HIGHZ,value);sv_note("SV_RECEIVER_PASS\n");return 0;
}
static int stream(void){
 SDCan frame={0x123,0,0,8,{0}};uint8_t bits[160];size_t n=0;CHECK(sd_can_encode(&frame,bits,sizeof bits,&n)==SD_OK);unsigned splits=0;
 for(size_t k=0;k<=n;k++){
  SvStream s={0};CnDecoded out={0};size_t used=99;CHECK(sv_stream_reset(&s,1)==SV_OK);int r=sv_stream_feed(&s,1,bits,k,&used,&out);CHECK(used==k&&r==(k==n?SV_FRAME:SV_MORE));
  SvStream before=s;CnDecoded old=out;used=99;CHECK(sv_stream_feed(&s,2,bits+k,n-k,&used,&out)==SV_STALE&&used==99&&!memcmp(&s,&before,sizeof s)&&!memcmp(&out,&old,sizeof out));
  if(k<n)CHECK(sv_stream_feed(&s,1,bits+k,n-k,&used,&out)==SV_FRAME&&out.frame.identifier==0x123);
  CHECK(sv_stream_reset(&s,1)==SV_STALE&&sv_stream_reset(&s,2)==SV_OK);
  used=99;CHECK(sv_stream_feed(&s,1,bits,n,&used,&out)==SV_STALE&&used==99);
  CHECK(sv_stream_feed(&s,2,bits,n,&used,&out)==SV_FRAME&&used==n);
  CHECK(sv_stream_reset(&s,UINT32_MAX)==SV_OK&&sv_stream_reset(&s,1)==SV_STALE);splits++;
 }
 SvStream s={0};CnDecoded out={0};size_t used;CHECK(sv_stream_reset(&s,1)==SV_OK&&sv_stream_feed(&s,1,bits,6,&used,&out)==SV_MORE);
 CHECK(sv_stream_reset(&s,2)==SV_OK&&sv_stream_feed(&s,2,bits,n,&used,&out)==SV_FRAME);
 values("stream",(uint32_t)n,splits,s.generation,out.frame.identifier);sv_note("SV_STREAM_PASS\n");return 0;
}
static int policy(void){
 SvPolicy out={99,99};CHECK(sv_policy(1,0,0,0,1,1,&out)==SV_OK&&out.action==SV_ACCEPT);
 CHECK(sv_policy(1,1,0,0,0,1,&out)==SV_OK&&out.action==SV_FORM_ERROR);
 CHECK(sv_policy(1,0,1,1,0,1,&out)==SV_OK&&out.action==SV_PROTOCOL_EXCEPTION);
 CnFault state={.tec=126,.rec=127,.state=CN_ACTIVE,.enabled=1};CnFault before=state;
 CHECK(out.action==SV_PROTOCOL_EXCEPTION&&!memcmp(&state,&before,sizeof state));
 CHECK(sv_policy(0,0,1,0,1,1,&out)==SV_OK&&out.action==SV_PROTOCOL_EXCEPTION);
 CHECK(sv_policy(0,0,1,0,0,0,&out)==SV_OK&&out.action==SV_ACCEPT&&out.overload==1);
 CHECK(sv_policy(0,1,1,0,0,0,&out)==SV_OK&&out.action==SV_FORM_ERROR&&out.overload==0);
 CnEvent e={.kind=CN_RECEIVER_ERROR};CnEffect effect;CHECK(cn_fault(&state,&e,&effect)==CN_OK&&state.rec==128&&state.state==CN_PASSIVE);
 out=(SvPolicy){99,98};CHECK(sv_policy(2,0,1,0,0,1,&out)==SV_INVALID&&out.action==99&&out.overload==98);
 CHECK(sv_policy(0,0,0,0,0,1,&out)==SV_INVALID&&out.action==99&&out.overload==98);
 values("policy",before.rec,state.rec,SV_PROTOCOL_EXCEPTION,SV_ACCEPT);sv_note("SV_POLICY_PASS\n");return 0;
}
static int bank(void){
 SDCan a={0x120,0,0,0,{0}},b={0x123,0,0,0,{0}};SvBank filter;CHECK(sv_bank(0,&a,0,0x7f0,&filter)==SV_OK&&filter.first==0x24000000&&filter.second==0xfe000006);
 unsigned count=0,accepted=0;for(unsigned id=0;id<2048;id++)for(unsigned remote=0;remote<2;remote++){b.identifier=id;b.remote=(uint8_t)remote;CHECK(sv_filter(&filter,&b,&accepted)==SV_OK&&accepted==(!remote&&(id&0x7f0)==0x120));count+=accepted;}
 b=(SDCan){0x120u<<18,1,0,0,{0}};CHECK(sv_filter(&filter,&b,&accepted)==SV_OK&&!accepted);
 a.identifier=0x123;b.identifier=(0x123u<<18)|7u;CHECK(sv_bank(1,&a,&b,0,&filter)==SV_OK&&sv_filter(&filter,&a,&accepted)==SV_OK&&accepted&&sv_filter(&filter,&b,&accepted)==SV_OK&&accepted);
 b.remote=1;CHECK(sv_filter(&filter,&b,&accepted)==SV_OK&&!accepted);
 /* Actual old and changed producers: lost oldest input cannot be recreated. */
 unsigned old_first[2],new_first[2];
 for(unsigned h=0;h<2;h++){SDFifo old={0};CnQueue changed={0};uint32_t values0[]={h?9u:0u,1,2,3};
  for(unsigned i=0;i<4;i++){CnRx v={0};v.frame.identifier=values0[i];CHECK(sd_can_push(&old,values0[i])==SD_OK&&cn_queue_put(&changed,&v)==CN_OK);}
  uint32_t first;CnRx first_new;CHECK(sd_can_pop(&old,&first)==SD_OK&&cn_queue_get(&changed,&first_new)==CN_FRAME);old_first[h]=first;new_first[h]=first_new.frame.identifier;
 }
 CHECK(old_first[0]!=old_first[1]&&new_first[0]==new_first[1]&&new_first[0]==1);
 values("bank",count,2,old_first[1],new_first[0]);sv_note("SV_BANK_PASS\n");return 0;
}
int sv_test(unsigned n){return n==0?baud():n==1?word():n==2?capture():n==3?receiver():n==4?stream():n==5?policy():n==6?bank():41;}
