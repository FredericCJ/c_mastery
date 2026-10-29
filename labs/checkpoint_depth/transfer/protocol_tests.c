#include "protocol.h"
#include "checkpoint_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPX_CHECK: %s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
static int same(const struct cpx_message *a,const struct cpx_message *b){return a->version==b->version && a->flags==b->flags && a->sequence==b->sequence && a->stamp==b->stamp && a->length==b->length && !memcmp(a->payload,b->payload,a->length);}
static void fixed(void){
 unsigned char bytes[CPX_WIRE_MAX+16],inplace[CPX_WIRE_MAX];size_t written,other,used;
 for(unsigned version=1;version<=2;++version)for(unsigned flags=0;flags<version;++flags)for(size_t n=0;n<=CPX_MAX_PAYLOAD;++n){
  struct cpx_message message={0};message.version=version;message.flags=flags;message.sequence=258;message.stamp=flags?UINT32_C(0x11223344):0;message.length=n;
  for(size_t i=0;i<n;++i)message.payload[i]=(unsigned char)(i*13+1);
  CHECK(cpx_protocol_encode(&message,bytes+1,CPX_WIRE_MAX,&written)==CPX_P_OK);
  /* Independent big-endian host representation: preserve the numeric value. */
  unsigned char host_be[2]={1,2};unsigned numeric=(unsigned)host_be[0]*256+host_be[1];CHECK(numeric==258 && bytes[3]==2 && bytes[4]==1);
  memcpy(inplace,message.payload,n);CHECK(cpx_protocol_inplace(inplace,sizeof inplace,n,version,flags,258,message.stamp,&other)==CPX_P_OK && other==written && !memcmp(inplace,bytes+1,written));
  struct cpx_parser parser;CHECK(cpx_protocol_init(&parser,CPX_MAX_PAYLOAD)==CPX_P_OK);
  CHECK(cpx_protocol_feed(&parser,bytes+1,written,&used)==CPX_P_FRAME && used==written && same(&parser.message,&message));
  if(version==2){struct cp_parser old;cp_parser_init(&old);CHECK(cp_feed(&old,bytes+1,written,&used)==CP_REJECT && used==2 && !old.ready);}
 }
 struct cpx_message m={0};m.version=2;m.flags=1;m.sequence=9;m.stamp=42;m.length=3;memcpy(m.payload,"abc",3);
 CHECK(cpx_protocol_encode(&m,bytes,sizeof bytes,&written)==CPX_P_OK);
 unsigned char saved[sizeof inplace];memset(inplace,0xcc,sizeof inplace);memcpy(saved,inplace,sizeof saved);other=99;
 CHECK(cpx_protocol_inplace(inplace,1,3,2,1,9,42,&other)==CPX_P_FULL && other==99 && !memcmp(saved,inplace,sizeof saved));
 struct cpx_parser parser;CHECK(cpx_protocol_init(&parser,2)==CPX_P_OK && cpx_protocol_feed(&parser,bytes,written,&used)==CPX_P_REJECT && used==6);
 for(size_t capacity=1;capacity<=3;capacity+=2){
  struct cpx_queue q;CHECK(cpx_protocol_queue(&q,capacity)==CPX_P_OK && cpx_protocol_init(&parser,CPX_MAX_PAYLOAD)==CPX_P_OK);
  for(size_t i=0;i<=capacity;++i){CHECK(cpx_protocol_feed(&parser,bytes,written,&used)==CPX_P_FRAME && used==written);if(i<capacity)CHECK(cpx_protocol_deliver(&parser,&q)==CPX_P_OK);}
  CHECK(cpx_protocol_deliver(&parser,&q)==CPX_P_FULL && parser.ready);
  CHECK(cpx_protocol_feed(&parser,bytes,written,&used)==CPX_P_FRAME && used==0);
  struct cpx_message got;CHECK(cpx_protocol_take(&q,&got)==CPX_P_OK && same(&got,&m));CHECK(cpx_protocol_deliver(&parser,&q)==CPX_P_OK && q.count==capacity);
  while(q.count)CHECK(cpx_protocol_take(&q,&got)==CPX_P_OK && same(&got,&m));
 }
}
static void print_message(const struct cpx_message *m){printf("%u:%u:%u:",m->version,(unsigned)m->sequence,(unsigned)m->stamp);for(size_t i=0;i<m->length;++i)printf("%02x",m->payload[i]);putchar('\n');}
static int oracle(int argc,char **argv){
 if(argc!=7)return 2;
 size_t chars=strlen(argv[2]),chunk=(size_t)strtoul(argv[3],NULL,10),limit=(size_t)strtoul(argv[4],NULL,10),capacity=(size_t)strtoul(argv[5],NULL,10),schedule=strlen(argv[6]);
 if(chars%2 || chars>512 || !chunk || chunk>256 || schedule>256)return 2;
 unsigned char input[256];size_t n=chars/2;for(size_t i=0;i<n;++i){unsigned b;if(sscanf(argv[2]+2*i,"%2x",&b)!=1)return 2;input[i]=(unsigned char)b;}
 struct cpx_parser parser;struct cpx_queue q;CHECK(cpx_protocol_init(&parser,limit)==CPX_P_OK && cpx_protocol_queue(&q,capacity)==CPX_P_OK);size_t offset=0;
 for(size_t step=0;step<10000;++step){
  if(step>=schedule || argv[6][step]!='0'){struct cpx_message m;if(cpx_protocol_take(&q,&m)==CPX_P_OK)print_message(&m);}
  size_t count=n-offset;if(count>chunk)count=chunk;size_t used;enum cpx_protocol_status status=cpx_protocol_feed(&parser,input+offset,count,&used);CHECK(used<=count);offset+=used;
  if(status==CPX_P_FRAME){enum cpx_protocol_status delivered=cpx_protocol_deliver(&parser,&q);CHECK(delivered==CPX_P_OK || delivered==CPX_P_FULL);}
  else CHECK(status==CPX_P_MORE || status==CPX_P_REJECT);
  if(offset==n && status==CPX_P_MORE && !q.count && !parser.ready)return 0;
 }
 CHECK(0);return 1;
}
int main(int argc,char **argv){if(argc>1 && !strcmp(argv[1],"oracle"))return oracle(argc,argv);fixed();puts("CPX_PASS version evolution, in-place encoding, bounded configurable parsing and queue refusal");return 0;}
