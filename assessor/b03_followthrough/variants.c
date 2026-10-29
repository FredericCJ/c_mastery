#include "follow.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VARIANT_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static uint64_t hash=UINT64_C(14695981039346656037);
static void add(unsigned n){hash=(hash^n)*UINT64_C(1099511628211);}
struct bf_link {unsigned calls;long result;};
static long link_write(void*v,const unsigned char*p,size_t n){struct bf_link*l=v;(void)p;(void)n;l->calls++;return l->result;}
static int scheduler(void){
 unsigned q,first,n,mode,count=0;static const unsigned char data[6]={1,2,3,4,5,6};static const long responses[]={-1,-2,0,1};
 for(q=1;q<=64;q++)for(first=0;first<2;first++)for(n=0;n<=5;n++)for(mode=0;mode<4;mode++){
  struct bf_round r={{{data,6,0,0},{data,n,0,0}},first};struct bf_link hot={0,responses[mode]},cold={0,1};b3_write_fn f[2]={link_write,link_write};void*c[2]={&hot,&cold};unsigned turn=0,chosen=9;
  do{unsigned oldhot=hot.calls,oldcold=cold.calls;int rc=bf_turn(&r,f,c,q,&chosen);CHECK(rc==B3_OK||rc==B3_WAIT||rc==B3_YIELD);CHECK(chosen==((first+turn)%2));CHECK(hot.calls-oldhot<=q&&cold.calls-oldcold<=q);turn++;CHECK(turn<=2*(n?n:1));}while(!r.writer[1].terminal);
  CHECK(r.writer[1].position==n&&cold.calls==n);count++;
 }
 {struct bf_round r={{{data,6,0,0},{data,1,0,0}},0};struct bf_link a={0,-1},b={0,1};b3_write_fn f[2]={link_write,link_write};void*c[2]={&a,&b};unsigned selected=9;CHECK(bf_turn(&r,f,c,0,&selected)==B3_YIELD&&selected==0);CHECK(bf_turn(&r,f,c,0,&selected)==B3_YIELD&&selected==1&&!a.calls&&!b.calls);}
 printf("VAR_SCHEDULER_PASS cases=%u hot_responses=4 quotas=64\n",count);return 0;
}
static int cancel(void){
 unsigned len,words=1,word,g,eg,count=0;
 for(len=0;len<=6;len++,words*=5)for(word=0;word<words;word++)for(g=1;g<=3;g++)for(eg=0;eg<=4;eg++){
  struct bf_operation o={g,BF_ACTIVE,1,g-1};unsigned k=word,i;
  for(i=0;i<len;i++){int rc=bf_event(&o,eg,(enum bf_event)(k%5));k/=5;add((unsigned)rc);add(o.generation);add(o.phase);add(o.owned);add(o.reports);}count++;
 }
 {struct bf_operation o={0};unsigned i;for(i=1;i<=3;i++){CHECK(bf_start(&o)==B3_OK);CHECK(bf_start(&o)==B3_FAILED);CHECK(bf_event(&o,i,BF_QUIESCENT)==B3_STALE);CHECK(bf_event(&o,i,BF_COMPLETE)==B3_OK);CHECK(bf_event(&o,i,BF_QUIESCENT)==B3_STALE);}CHECK(bf_start(&o)==B3_FAILED&&o.reports==3);}
 printf("VAR_CANCEL %u %" PRIu64 "\n",count,hash);return 0;
}
static int message(void){
 unsigned n,words=1,word,q,lose,count=0;
 for(n=0;n<=6;n++,words*=3)for(word=0;word<words;word++)for(q=0;q<=6;q++)for(lose=0;lose<=1;lose++){
  struct bf_message m={0};unsigned char bytes[6],bad[6];unsigned i,k=word,out=999;int rc;
  for(i=0;i<n;i++){bytes[i]=(unsigned char)(k%3);bad[i]=bytes[i];k/=3;}
  rc=bf_message(&m,1,bytes,n,q,(int)lose,&out);add((unsigned)rc);add(out);add(m.done);add(m.total);add(m.effects);add(m.complete);
  if(n){bad[0]=(unsigned char)(bytes[0]+1);CHECK(bf_message(&m,1,bad,n,6,0,&out)==B3_FAILED);}
  rc=bf_message(&m,1,bytes,n,6,1,&out);add((unsigned)rc);add(out);add(m.done);add(m.total);add(m.effects);add(m.complete);
  rc=bf_message(&m,1,bytes,n,6,0,&out);add((unsigned)rc);add(out);add(m.done);add(m.total);add(m.effects);add(m.complete);
  CHECK(bf_message(&m,3,bytes,n,6,0,&out)==B3_STALE);count++;
 }
 {struct bf_message m={0};unsigned out=99,i;unsigned char p=1;for(i=1;i<=3;i++)CHECK(bf_message(&m,i,&p,1,1,0,&out)==B3_OK&&out==i);CHECK(bf_message(&m,1,&p,1,1,0,&out)==B3_STALE&&out==3);CHECK(bf_message(&m,4,&p,1,1,0,&out)==B3_INVALID);}
 printf("VAR_MESSAGE %u %" PRIu64 "\n",count,hash);return 0;
}
static int device(void){
 unsigned s,m,a,b,count=0;
 for(s=0;s<16;s++)for(m=0;m<32;m++)for(a=0;a<16;a++)for(b=0;b<16;b++){
  struct bf_register r={s|a,0,0,0};unsigned out=999;int rc=bf_w0c(&r,m);r.status|=b;
  add((unsigned)rc);add(r.status);add(r.writes);add(r.reads);add(r.last_write);
  CHECK(bf_clear_read(&r,&out)==B3_OK);add(out);add(r.status);add(r.reads);
  CHECK(bf_clear_read(&r,&out)==B3_OK&&out==0&&r.reads==2);count++;
 }
 printf("VAR_DEVICE %u %" PRIu64 "\n",count,hash);return 0;
}
struct bf_counter {unsigned counter,pattern,at,last_low,reads;};
static unsigned half(void*v,unsigned high){struct bf_counter*c=v;unsigned result=high?(c->counter>>8):(c->counter&255);
 if(!high)c->last_low=c->counter;
 c->counter=(c->counter+((c->pattern>>c->at)&1u))&65535u;c->at++;c->reads++;return result;
}
static unsigned unstable(void*v,unsigned high){unsigned *n=v;unsigned r=(*n)++/3;return high?r+((*n)%3==0):0;}
static unsigned badhalf(void*v,unsigned high){(void)v;(void)high;return 256;}
static int clock_cases(void){
 unsigned start,pattern,count=0,ms,num,den;
 for(start=0;start<65536;start++)for(pattern=0;pattern<64;pattern++){
  struct bf_counter c={start,pattern,0,0,0};uint16_t out=999;
  CHECK(bf_coherent(half,&c,2,&out)==B3_OK&&out==c.last_low&&(c.reads==3||c.reads==6));count++;
 }
 {unsigned n=0;uint16_t out=999;CHECK(bf_coherent(unstable,&n,2,&out)==B3_WAIT&&out==999&&n==6);CHECK(bf_coherent(badhalf,NULL,1,&out)==B3_FAILED&&out==999);CHECK(bf_coherent(half,NULL,0,&out)==B3_WAIT&&out==999);CHECK(bf_coherent(NULL,NULL,1,&out)==B3_INVALID);}
 for(ms=0;ms<=1000;ms++)for(num=1;num<=9;num++)for(den=1;den<=9;den++){
  unsigned out=999;CHECK(bf_ticks(ms,num,den,&out)==B3_OK);CHECK((uint64_t)out*num>=(uint64_t)ms*den);CHECK(!out||(uint64_t)(out-1)*num<(uint64_t)ms*den);
 }
 {unsigned out=999;CHECK(bf_ticks(UINT_MAX,1,2,&out)==B3_INVALID&&out==999);CHECK(bf_ticks(1,0,1,&out)==B3_INVALID&&out==999);}
 for(start=0;start<65536;start++){
  static const unsigned e[]={0,1,253,254,255,32767,32768,65535};unsigned i,j;
  for(i=0;i<sizeof e/sizeof *e;i++)for(j=0;j<sizeof e/sizeof *e;j++){
   int out=9,rc=bf_due16((uint16_t)(start+e[i]),(uint16_t)start,(uint16_t)e[j],&out);
   CHECK(rc==(e[j]>=32768?B3_INVALID:e[i]>=32768?B3_AMBIGUOUS:B3_OK));CHECK(out==((e[j]>=32768||e[i]>=32768)?9:(e[i]>=e[j])));
  }
 }
 printf("VAR_CLOCK_PASS coherent=%u conversion=81081 width=4194304\n",count);return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"scheduler"))return scheduler();if(!strcmp(argv[1],"cancel"))return cancel();if(!strcmp(argv[1],"message"))return message();if(!strcmp(argv[1],"device"))return device();if(!strcmp(argv[1],"clock"))return clock_cases();return 2;}
