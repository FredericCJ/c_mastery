#include "flow.h"
#include "http_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"HF_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static const char *authority="localhost:8080", *date="Sun, 01 Jan 2023 00:00:00 GMT";
static const char get[]="GET /health HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";
static int lifecycle(void) {
  HFPool p={0};HFHandle h[5];int fd=-99;
  for(unsigned i=0;i<4;i++)CHECK(!hf_admit(&p,11+(int)i,3,&h[i]));
  CHECK(hf_admit(&p,15,3,&h[4])==1);CHECK(!hf_pin(&p,h[1]));CHECK(!hf_pin(&p,h[1]));
  CHECK(!hf_close(&p,h[1],&fd) && fd==12);CHECK(hf_close(&p,h[1],&fd)==-1);
  CHECK(hf_admit(&p,12,3,&h[4])==1);fd=-99;
  CHECK(hf_callback(&p,h[1],&fd)==1 && fd==-99);CHECK(hf_admit(&p,16,3,&h[4])==1);
  CHECK(hf_callback(&p,h[1],&fd)==1 && fd==-99);CHECK(!hf_admit(&p,12,3,&h[4]));
  CHECK(h[4].index==h[1].index && h[4].generation!=h[1].generation);
  CHECK(hf_callback(&p,h[1],&fd)==-1);CHECK(!hf_pin(&p,h[4]));CHECK(!hf_callback(&p,h[4],&fd) && fd==12);
  CHECK(hf_admit(&p,19,0,&h[4])==-1);CHECK(hf_admit(&p,11,3,&h[4])==-1);
  HFPool q={0};q.slot[0].generation=UINT32_MAX;CHECK(!hf_admit(&q,7,3,&h[4]) && h[4].index==1 && q.slot[0].retired);
  puts("HF_PASS:lifecycle:4-live,fifth-refused,close-once,2-pins-quiescent,recycled-fd-stale,generation-retired");return 0;
}
typedef struct {int script[4],bad,closed[4];unsigned call,nclose;} AcceptScript;
static int accept_script(void *v){AcceptScript *s=v;return s->script[s->call++];}
static int flag_script(void *v,int fd){return fd==((AcceptScript *)v)->bad;}
static void close_script(void *v,int fd){AcceptScript *s=v;s->closed[s->nclose++]=fd;}
static int admission(void) {
  HFPool p={0};HFHandle handles[4];unsigned count=99;AcceptScript s={{-1,21,22,-2},22,{0},0,0};
  HFAccept ops={accept_script,flag_script,close_script,&s};CHECK(!hf_accept_batch(&p,ops,handles,&count));
  CHECK(s.call==4 && count==1 && p.slot[handles[0].index].fd==21 && s.nclose==1 && s.closed[0]==22);
  s=(AcceptScript){{-2,0,0,0},0,{0},0,0};CHECK(!hf_accept_batch(&p,ops,handles,&count) && count==0 && s.call==1);
  puts("HF_PASS:admission:EINTR-retry,EAGAIN-race,flags-failure-close,publication-after-flags");return 0;
}
typedef struct {int results[16];unsigned count,call;size_t offsets[16],n;unsigned char bytes[6400];} Sink;
static int sink(void *v,const void *s,size_t n) {
  Sink *p=v;p->offsets[p->call]=p->n;int r=p->call<p->count?p->results[p->call]:(int)n;p->call++;
  if(r>0 && (size_t)r<=n){memcpy(p->bytes+p->n,s,(size_t)r);p->n+=(size_t)r;}
  return r;
}
static int io(void) {
  Sink s={{4,-1,2,-2,3},5,0,{0},0,{0}};size_t off=0;const char bytes[]="123456789";
  CHECK(http_drain(sink,&s,bytes,9,&off)==0 && off==6 && s.call==4);
  CHECK(http_drain(sink,&s,bytes,9,&off)==1 && off==9 && !memcmp(s.bytes,bytes,9));
  const size_t offsets[]={0,4,4,6,6};CHECK(!memcmp(s.offsets,offsets,sizeof offsets));
  for(int bad=0;bad<3;bad++){Sink t={0};t.count=1;t.results[0]=bad==0?0:bad==1?10:-3;off=0;CHECK(http_drain(sink,&t,bytes,9,&off)==-1 && off==0);}
  puts("HF_PASS:io:offsets4,4,6,6,9;zero,oversize,EPIPE-abort;immutable");return 0;
}
static int response(void) {
  HttpRequest r={0};r.status=404;r.head=1;strcpy(r.target,"/unknown");unsigned char out[1600];size_t n=0;
  CHECK(!hf_response(&r,date,0,0,0,out,sizeof out-1,&n));out[n]=0;
  CHECK(strstr((char *)out,"Content-Length: 10\r\n") && !memcmp(out+n-4,"\r\n\r\n",4));
  r.status=405;strcpy(r.target,"/echo");CHECK(!hf_response(&r,0,0,0,0,out,sizeof out-1,&n));out[n]=0;
  CHECK(strstr((char *)out,"Content-Length: 19\r\n") && strstr((char *)out,"Allow: POST\r\n") && !strstr((char *)out,"Date:"));
  const unsigned char utf8[]={0xc3,0xa9,'\n'};r.head=0;
  CHECK(!hf_response(&r,0,0,utf8,sizeof utf8,out,sizeof out-1,&n));out[n]=0;CHECK(strstr((char *)out,"Content-Length: 3\r\n") && !memcmp(out+n-3,utf8,3));
  memset(out,0xa5,sizeof out);n=77;CHECK(hf_response(&r,date,0,0,0,out,8,&n)==-1 && n==77 && out[0]==0xa5);
  CHECK(hf_response(&r,"injected\r\n",0,0,0,out,sizeof out,&n)==-1 && n==77);
  char d[30];size_t z=99;CHECK(!hd_date(1,19358,0,d,sizeof d,&z) && z==29 && !strcmp(d,date));
  d[0]='!';CHECK(!hd_date(0,0,0,d,sizeof d,&z) && z==0 && d[0]=='!');
  puts("HF_PASS:response:HEAD10,19;Allow;UTF8-three-octets;Date-fixture-and-omission;transactional-capacity");return 0;
}
static int fifo(unsigned cap) {
  HFFlow f;CHECK(!hf_init(&f,cap,authority,0));char input[512];
  int z=snprintf(input,sizeof input,"%sPOST /echo HTTP/1.1\r\nHost: %s\r\nContent-Length: 1\r\nExpect: 100-continue\r\n\r\nx",get,authority);
  CHECK(z>0 && (size_t)z<sizeof input);size_t off=0;Sink s={0};unsigned interim=0,final=0;
  for(unsigned round=0;round<8 && (off<(size_t)z || f.count);round++) {
    size_t used;CHECK(!hf_feed(&f,input+off,(size_t)z-off,&used,1,authority,date));off+=used;
    if(f.count){HFOutput *o=&f.output[f.head];if(o->interim){CHECK(final==1 && o->request==2);interim++;}else {final++;CHECK(o->request==final);}CHECK(hf_write(&f,sink,&s,2)==1);}
  }
  CHECK(off==(size_t)z && interim==1 && final==2 && f.requests==2);s.bytes[s.n]=0;
  char *a=strstr((char *)s.bytes,"100 Continue"),*b=strstr((char *)s.bytes,"200 OK");CHECK(a && b && b<a && strstr(a,"200 OK"));
  printf("HF_PASS:fifo%u:first-final,second-100,second-final;bounded-parse-resumption\n",cap);return 0;
}
static int closing(void) {
  HFFlow f;CHECK(!hf_init(&f,2,authority,0));char input[1024];size_t u;Sink s={0};
  int n=snprintf(input,sizeof input,"%sGET /health HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n%s",get,authority,get);
  CHECK(n>0);CHECK(!hf_feed(&f,input,(size_t)n,&u,1,authority,date));CHECK(f.requests==2 && f.closing && u==(size_t)n-strlen(get));
  while(f.count)CHECK(hf_write(&f,sink,&s,2)==1);
  CHECK(hf_tick(&f,2,date)==1 && hf_tick(&f,1001,date)==0 && hf_tick(&f,1002,date)==2);
  CHECK(!hf_feed(&f,get,strlen(get),&u,3,authority,date) && u==0 && f.requests==2);
  CHECK(!hf_init(&f,2,authority,0));for(unsigned i=0;i<5;i++){CHECK(!hf_feed(&f,get,strlen(get),&u,1,authority,date));if(i<4){CHECK(u==strlen(get));CHECK(hf_write(&f,sink,&s,2)==1);}else CHECK(u==0);}
  CHECK(f.requests==4 && f.closing);puts("HF_PASS:closing:request2-close-drops3;maximum4;shutdown-then1000ms-drain");return 0;
}
static int prefix_fifo(void) {
  HFFlow f;CHECK(!hf_init(&f,2,authority,0));size_t u;CHECK(!hf_feed(&f,get,strlen(get),&u,1,authority,date));
  Sink s={{4,-2},2,0,{0},0,{0}};CHECK(hf_write(&f,sink,&s,2)==0 && f.output[f.head].offset==4);
  const char header[]="POST /echo HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: 1\r\nExpect: 100-continue\r\n\r\n";
  CHECK(!hf_feed(&f,header,sizeof header-1,&u,3,authority,date));CHECK(f.count==2 && f.output[(f.head+1)%2].interim);
  CHECK(hf_write(&f,sink,&s,4)==1);CHECK(hf_write(&f,sink,&s,4)==1);
  CHECK(!hf_feed(&f,"x",1,&u,5,authority,date) && u==1);CHECK(hf_write(&f,sink,&s,6)==1);s.bytes[s.n]=0;
  char *first=strstr((char *)s.bytes,"200 OK"),*interim=strstr((char *)s.bytes,"100 Continue");CHECK(first && interim && first<interim && strstr(interim,"200 OK"));
  puts("HF_PASS:prefix-fifo:4-byte-prefix,immutable-first,ordered-second100,second-final");return 0;
}
static int deadlines(void) {
  CHECK(http_expired(15000,0,14900,15000,5000));CHECK(!http_expired(14999,0,14900,15000,5000));CHECK(http_expired(4,5,5,15000,5000));
  CHECK(!http_expired(UINT64_MAX,UINT64_MAX-14999,UINT64_MAX-4999,15000,5000));
  CHECK(http_expired(UINT64_MAX,UINT64_MAX-15000,UINT64_MAX-4999,15000,5000));
  CHECK(http_expired(3,UINT64_MAX-2,UINT64_MAX-1,15000,5000));
  HFFlow f;CHECK(!hf_init(&f,2,authority,0));f.progress=14900;CHECK(hf_tick(&f,15000,date)==0 && f.count==1 && f.parser.request.status==408 && f.closing);
  CHECK(!hf_init(&f,2,authority,0));size_t u;CHECK(!hf_feed(&f,get,strlen(get),&u,1,authority,date));CHECK(hf_tick(&f,15001,date)==3 && f.aborted && !f.draining);
  CHECK(!hf_init(&f,1,authority,100));CHECK(hf_tick(&f,99,date)==3);
  puts("HF_PASS:deadlines:total-not-renewed,>=,uint64-subtraction,regression,408,output-abort-separate");return 0;
}
static int eof_events(void) {
  for(unsigned complete=0;complete<2;complete++) {
    HFFlow f;CHECK(!hf_init(&f,2,authority,0));const char req[]="POST /echo HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: 5\r\n\r\nabcde";size_t u;
    CHECK(!hf_feed(&f,req,sizeof req-2+complete,&u,1,authority,date));HFFlow old=f;
    CHECK(!hf_read_event(&f,-1,2,date) && !memcmp(&f,&old,sizeof f));CHECK(!hf_read_event(&f,-2,3,date) && !memcmp(&f,&old,sizeof f));
    CHECK(!hf_read_event(&f,0,4,date) && f.closing && f.count==1);
    CHECK(!memcmp(f.output[f.head].bytes,complete?"HTTP/1.1 200":"HTTP/1.1 400",12));
  }
  puts("HF_PASS:eof-events:final-byte,EINTR,EAGAIN,real-EOF;complete200,incomplete400");return 0;
}
int main(int argc,char **argv) {
  if(argc==2 && !strcmp(argv[1],"ledger")){printf("{\"HttpRequest\":%zu,\"HttpParser\":%zu,\"HFOutput\":%zu,\"HFFlow\":%zu,\"HFPool\":%zu,\"four_flows\":%zu,\"line\":%zu,\"body\":%zu,\"target\":%zu,\"output_bytes\":%zu}\n",sizeof(HttpRequest),sizeof(HttpParser),sizeof(HFOutput),sizeof(HFFlow),sizeof(HFPool),4*sizeof(HFFlow),sizeof(((HttpParser *)0)->line),sizeof(((HttpRequest *)0)->body),sizeof(((HttpRequest *)0)->target),sizeof(((HFFlow *)0)->output));return 0;}
  if(lifecycle() || admission() || io() || response() || fifo(2) || fifo(1) || prefix_fifo() || closing() || deadlines() || eof_events())return 1;
  puts("HF_PASS:all");return 0;
}
