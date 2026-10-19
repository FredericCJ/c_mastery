#include "format_stream.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MPO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
static MfpSpan span(const char *s){return (MfpSpan){(const unsigned char *)s,strlen(s)};}
static int parsing(void){MfpProgram p={0},old;MfpMessage m={{0},99};MfpArg args[2]={mfp_number(42),mfp_bytes(span("A"))};
 CHECK(mfp_parse(span("{{{u0}}}:{s1}:%n"),&p)==MFP_OK&&p.required[0]==MFP_NUMBER&&p.required[1]==MFP_BYTES);
 CHECK(mfp_prepare(&p,args,2,&m)==MFP_OK&&m.length==9&&!memcmp(m.bytes,"{42}:A:%n",9));
 old=p;CHECK(mfp_parse(span("x{"),&p)==MFP_FORMAT&&!memcmp(&p,&old,sizeof p));
 CHECK(mfp_parse(span("{u0}{s0}"),&p)==MFP_TYPE&&!memcmp(&p,&old,sizeof p));
 CHECK(mfp_parse(span("{n}"),&p)==MFP_FORMAT);CHECK(mfp_parse(span("{u4}"),&p)==MFP_FORMAT);CHECK(mfp_parse(span("}"),&p)==MFP_FORMAT);
 CHECK(mfp_parse((MfpSpan){NULL,0},&p)==MFP_OK&&mfp_prepare(&p,NULL,0,&m)==MFP_OK&&!m.length);
 puts("MFP_PARSE bounded-grammar copied-descriptors typed-placeholders");return 0;}
static int atomic_message(void){MfpProgram p;MfpMessage m={{77},99},old=m;unsigned char b[16];MfpArg a=mfp_bytes(span("x"));MPSink s={b,11,2};
 memset(b,0xa5,sizeof b);CHECK(mfp_parse(span("{u0}"),&p)==MFP_OK);CHECK(mfp_prepare(&p,&a,1,&m)==MFP_TYPE&&!memcmp(&m,&old,sizeof m));
 a=mfp_number(UINT32_MAX);CHECK(mfp_render(&p,&a,1,&s)==MFP_FULL&&s.used==2);for(size_t i=0;i<sizeof b;++i)CHECK(b[i]==0xa5);
 s.capacity=12;CHECK(mfp_render(&p,&a,1,&s)==MFP_OK&&s.used==12&&!memcmp(b+2,"4294967295",10)&&b[12]==0xa5);
 unsigned char text[]={0,'%','n'};a=mfp_bytes((MfpSpan){text,3});CHECK(mfp_parse(span("{s0}"),&p)==MFP_OK&&mfp_prepare(&p,&a,1,&m)==MFP_OK&&m.length==3&&!memcmp(m.bytes,text,3));text[1]='X';CHECK(m.bytes[1]=='%');
 unsigned evaluations=0;a=MFP_ARG((++evaluations,(uint32_t)9));CHECK(evaluations==1&&a.number==9);
 puts("MFP_ATOMIC complete-message-before-sink capacity-refusal-unchanged");return 0;}
typedef struct {MfpWriteResult result;unsigned calls;unsigned char bytes[512];size_t used;} Writer;
static MfpWriteResult write_part(void *p,const unsigned char *b,size_t n){Writer *w=p;size_t k=w->result.accepted<n?w->result.accepted:n;++w->calls;for(size_t i=0;i<k&&w->used<sizeof w->bytes;++i)w->bytes[w->used++]=b[i];return w->result;}
static int streaming(void){MfpMessage m={{'a','b','c'},3};MfpStream s={0};Writer w={{1,MFP_WRITE_AGAIN},0,{0},0};
 CHECK(mfp_start(&s,&m)==MFP_OK);m.bytes[0]='X';CHECK(mfp_step(&s,write_part,&w)==MFP_AGAIN&&s.offset==1&&w.bytes[0]=='a');
 CHECK(mfp_start(&s,&m)==MFP_BUSY&&s.offset==1);w.result=(MfpWriteResult){2,MFP_WRITE_OK};CHECK(mfp_step(&s,write_part,&w)==MFP_OK&&s.offset==3&&!memcmp(w.bytes,"abc",3));CHECK(mfp_step(&s,NULL,NULL)==MFP_OK&&w.calls==2);
 CHECK(mfp_start(&s,&m)==MFP_OK);w.result=(MfpWriteResult){1,MFP_WRITE_ERROR};CHECK(mfp_step(&s,write_part,&w)==MFP_IO&&s.offset==1&&s.state==MFP_FAILED&&!s.uncertain);unsigned calls=w.calls;CHECK(mfp_step(&s,write_part,&w)==MFP_IO&&w.calls==calls);
 CHECK(mfp_start(&s,&m)==MFP_OK);w.result=(MfpWriteResult){1,MFP_WRITE_OK};CHECK(mfp_step(&s,write_part,&w)==MFP_MORE);CHECK(mfp_cancel(&s)==MFP_CANCELLED&&s.offset==1);calls=w.calls;CHECK(mfp_step(&s,write_part,&w)==MFP_CANCELLED&&w.calls==calls);
 CHECK(mfp_start(&s,&m)==MFP_OK);w.result=(MfpWriteResult){4,MFP_WRITE_OK};CHECK(mfp_step(&s,write_part,&w)==MFP_PROTOCOL&&s.uncertain&&s.offset==0);calls=w.calls;CHECK(mfp_step(&s,write_part,&w)==MFP_PROTOCOL&&w.calls==calls);
 m.length=0;CHECK(mfp_start(&s,&m)==MFP_OK&&mfp_step(&s,NULL,NULL)==MFP_OK&&s.state==MFP_DONE);
 puts("MFP_STREAM accepted-prefix retry-offset terminal-error cancellation");return 0;}
static int base(void){unsigned char b[16];for(size_t cap=0;cap<=11;++cap){MPSink s={b,cap,0};memset(b,0xa5,sizeof b);int r=mp_format(&s,mp_arg_number(UINT32_MAX));CHECK(r==(cap>=10)&&s.used==(cap>=10?10:0));if(cap>=10)CHECK(!memcmp(b,"4294967295",10));for(size_t i=s.used;i<sizeof b;++i)CHECK(b[i]==0xa5);}
 MPSink s={b,16,0};CHECK(mp_format(&s,mp_arg_number(0))&&s.used==1&&b[0]=='0');CHECK(mp_format(&s,mp_arg_text("%n"))&&s.used==3&&b[1]=='%'&&b[2]=='n');puts("MFP_BASE decimal-ten-bound literal-text full-capacity-boundaries");return 0;}
static size_t unhex(const char *h,unsigned char *out){size_t n=!strcmp(h,"-")?0:strlen(h)/2;for(size_t i=0;i<n;++i){unsigned v=0;(void)sscanf(h+2*i,"%2x",&v);out[i]=(unsigned char)v;}return n;}
static void hex(const unsigned char *b,size_t n){if(!n)putchar('-');for(size_t i=0;i<n;++i)printf("%02x",b[i]);}
static int oracle(void){char op;while(scanf(" %c",&op)==1){
 if(op=='F'){char h[257];unsigned char source[128],text[4][128],output[160];MfpProgram p;MfpMessage m={{0},777};MfpArg args[4];size_t count,cap,used;if(scanf("%256s %zu %zu %zu",h,&count,&cap,&used)!=4||count>4||cap>160)return 2;size_t n=unhex(h,source);for(size_t i=0;i<count;++i){unsigned kind;uint32_t value;if(scanf("%u %" SCNu32 " %256s",&kind,&value,h)!=3)return 2;args[i]=(MfpArg){kind,value,{text[i],unhex(h,text[i])}};}memset(output,0xa5,sizeof output);MPSink sink={output,cap,used};int ps=mfp_parse((MfpSpan){source,n},&p),ms=-1,rs=-1;if(ps==MFP_OK){ms=mfp_prepare(&p,args,count,&m);rs=mfp_render(&p,args,count,&sink);}printf("%d %d %d %zu %zu ",ps,ms,rs,m.length,sink.used);hex(output,sizeof output);putchar('\n');}
 else if(op=='S'){char h[257];MfpMessage m={{0},0};MfpStream s={0};Writer w={{0,0},0,{0},0};size_t n;if(scanf("%256s %zu",h,&n)!=2||n>32)return 2;m.length=unhex(h,m.bytes);if(mfp_start(&s,&m)!=MFP_OK)return 2;for(size_t i=0;i<n;++i){unsigned cancel;if(scanf("%zu %u %u",&w.result.accepted,&w.result.status,&cancel)!=3)return 2;if(cancel)(void)mfp_cancel(&s);int r=mfp_step(&s,write_part,&w);printf("%s%d",i?",":"",r);}printf(" %u %zu %u %u ",s.state,s.offset,s.uncertain,w.calls);hex(w.bytes,w.used);putchar('\n');}
 else return 2;
 }return 0;}
static int metrics(void){MfpProgram p;MfpMessage m;MfpArg a=mfp_number(UINT32_MAX);double seconds[7];uint64_t checksum=0;CHECK(mfp_parse(span("value={u0}"),&p)==MFP_OK);for(unsigned sample=0;sample<7;++sample){clock_t start=clock();for(unsigned i=0;i<10000;++i){CHECK(mfp_prepare(&p,&a,1,&m)==MFP_OK);checksum+=m.length;}seconds[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":10000,\"checksum\":%" PRIu64 ",\"program_size\":%zu,\"stream_size\":%zu,\"seconds\":[",checksum,sizeof p,sizeof(MfpStream));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",seconds[i]);puts("]}");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"format_parse"))return parsing();if(!strcmp(argv[1],"format_atomic"))return atomic_message();if(!strcmp(argv[1],"format_stream"))return streaming();if(!strcmp(argv[1],"format_base"))return base();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
