#include "follow.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BASE_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static const unsigned char bytes[6]={11,23,37,43,59,61};
typedef struct {unsigned script[6],length,at,calls,offset[8],copied;unsigned char output[8];} Backend;
static long write_one(void *v,const unsigned char *p,size_t n){
 Backend *b=v;unsigned e=b->at<b->length?b->script[b->at++]:4;
 b->offset[b->calls++]=(unsigned)(p-bytes);
 if(e==0){if(n)b->output[b->copied++]=*p;return 1;}
 return e==1?-1:e==2?-2:e==3?0:-3;
}
static int io(void){
 unsigned sl,words=1,word,len,budget,total=0;
 for(sl=0;sl<=6;sl++,words*=5)for(word=0;word<words;word++)for(len=0;len<=5;len++)for(budget=1;budget<=3;budget++){
  Backend b={{0},sl,0,0,{0},0,{0}};B3Writer w={bytes,len,0,0};unsigned i,k=word,pos=0,terminal=0,at=0;
  for(i=0;i<sl;i++){b.script[i]=k%5;k/=5;}
  for(i=0;i<9;i++){
   unsigned used=0,offset[8]={0},old_calls=b.calls;int expected=B3_YIELD,got;
   if(terminal)expected=terminal==1?B3_OK:B3_FAILED;
   else if(pos==len){terminal=1;expected=B3_OK;}
   else while(used<budget){unsigned e=at<sl?b.script[at++]:4;offset[used++]=pos;
    if(e==0){pos++;if(pos==len){terminal=1;expected=B3_OK;break;}}
    else if(e==2||e==3){expected=B3_WAIT;break;}
    else if(e==4){terminal=2;expected=B3_FAILED;break;}
   }
   got=b3_pump(&w,write_one,&b,budget);
   CHECK(got==expected&&w.position==pos&&(unsigned)w.terminal==terminal&&b.calls-old_calls==used);
   for(k=0;k<used;k++)CHECK(b.offset[old_calls+k]==offset[k]);
   CHECK(b.copied==pos&&!memcmp(b.output,bytes,pos));
   if(terminal){unsigned calls=b.calls;CHECK(b3_pump(&w,write_one,&b,budget)==expected&&b.calls==calls);break;}
  }
  CHECK(terminal);total++;
 }
 {Backend b={{0},0,0,0,{0},0,{0}};B3Writer w={bytes,5,0,0};CHECK(b3_pump(&w,write_one,&b,0)==B3_YIELD&&b.calls==0);CHECK(b3_pump(&w,write_one,&b,65)==B3_INVALID&&b.calls==0);}
 printf("BASE_IO_PASS scripts=%u\n",total);return 0;
}
static long oversized(void *c,const unsigned char *p,size_t n){(void)c;(void)p;return (long)n+1;}
static int cancel(void){
 unsigned length,words=1,word,g,eg,total=0;
 for(length=0;length<=6;length++,words*=3)for(word=0;word<words;word++)for(g=1;g<=3;g++)for(eg=0;eg<=4;eg++){
  B3Operation o={g,B3_ACTIVE,1,g-1};unsigned phase=B3_ACTIVE,owned=1,reports=g-1,k=word,i;
  for(i=0;i<length;i++){enum B3Event e=(enum B3Event)(k%3);int expected=B3_STALE;k/=3;
   if(g==eg&&owned){if(e==B3_TIMEOUT){phase=B3_CANCELLING;expected=B3_WAIT;}else if(e==B3_COMPLETE||phase==B3_CANCELLING){phase=e==B3_COMPLETE?B3_COMPLETED:B3_CANCELLED;owned=0;reports++;expected=B3_OK;}}
   CHECK(b3_event(&o,eg,e)==expected&&o.phase==phase&&o.owned==owned&&o.reports==reports&&o.generation==g);
  }total++;
 }
 {B3Operation o={0};unsigned i;for(i=1;i<=3;i++){CHECK(b3_start(&o)==B3_OK&&o.generation==i);CHECK(b3_start(&o)==B3_FAILED);CHECK(b3_event(&o,i-1,B3_COMPLETE)==B3_STALE);CHECK(b3_event(&o,i,B3_COMPLETE)==B3_OK);}CHECK(b3_start(&o)==B3_FAILED&&o.reports==3);}
 printf("BASE_CANCEL_PASS words=%u\n",total);return 0;
}
static int mmio(void){
 unsigned s,m,before,after,count=0;
 for(s=0;s<16;s++)for(m=0;m<32;m++){
  B3Device d={s,71,0,0};unsigned out=999;int rc=b3_ack(&d,m);
  CHECK(rc==(m<16?B3_OK:B3_INVALID)&&d.status==(m<16?(s&~m):s)&&d.writes==(m<16)&&!d.reads);
  d.status=s;CHECK(b3_read_data(&d,&out)==((s&1)?B3_OK:B3_WAIT));CHECK(out==((s&1)?71u:999u)&&d.reads==(s&1)&&d.status==(s&~1u));
  CHECK(b3_read_data(&d,&out)==B3_WAIT&&d.reads==(s&1));count++;
 }
 for(s=0;s<16;s++)for(m=0;m<16;m++)for(before=0;before<16;before++)for(after=0;after<16;after++){
  B3Device d={s|before,0,0,0};CHECK(b3_ack(&d,m)==B3_OK);d.status|=after;
  CHECK(d.status==(((s|before)&~m)|after)&&d.writes==1&&!d.reads);count++;
 }
 {B3Device d={3,0,0,0};unsigned wrong=d.status|1;CHECK(b3_ack(&d,wrong)==B3_OK&&d.status==0);d.status=3;CHECK(b3_ack(&d,1)==B3_OK&&d.status==2);}
 printf("BASE_MMIO_PASS cases=%u\n",count);return 0;
}
static int deadline(void){
 unsigned s,e,d,valid=0,ambiguous=0,invalid=0;
 for(s=0;s<256;s++)for(e=0;e<256;e++)for(d=0;d<256;d++){
  int out=7;int got=b3_due((uint8_t)(s+e),(uint8_t)s,(uint8_t)d,&out);
  if(d>=128){CHECK(got==B3_INVALID&&out==7);invalid++;}
  else if(e>=128){CHECK(got==B3_AMBIGUOUS&&out==7);ambiguous++;}
  else{CHECK(got==B3_OK&&out==(e>=d));valid++;}
 }
 CHECK(b3_due(0,0,0,NULL)==B3_INVALID);
 printf("BASE_DEADLINE_PASS valid=%u ambiguous=%u invalid=%u\n",valid,ambiguous,invalid);return 0;
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"io")){B3Writer w={bytes,5,0,0};CHECK(b3_pump(&w,oversized,NULL,1)==B3_FAILED&&w.position==0&&w.terminal==2);return io();}
 if(!strcmp(argv[1],"cancel"))return cancel();
 if(!strcmp(argv[1],"mmio"))return mmio();
 if(!strcmp(argv[1],"deadline"))return deadline();
 return 2;
}
