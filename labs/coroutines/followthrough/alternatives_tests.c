#define _POSIX_C_SOURCE 200809L
#include "alternatives.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CX_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int iterator(void){unsigned cases=0;for(unsigned start=0;start<10;start++)for(unsigned end=start;end<33;end++)for(unsigned step=1;step<=8;step++)for(unsigned take=0;take<6;take++){
 COFlow fused={{start,end,step,0},take,0};CRUnfused other={.flow=fused};uint32_t x=91,y=91;unsigned produced=0;
 for(;;){int a=co_flow_next(&fused,&x),b=cr_unfused_next(&other,&y);C(a==b&&x==y&&fused.pulls==other.flow.pulls&&fused.source.current==other.flow.source.current);if(a==CO_DONE)break;C(a==0&&produced++<take);}
 C(other.maps==produced&&other.predicates<=other.flow.pulls);cases++;
 }printf("CX_ITERATOR:%u exact fused/unfused value and pull-count cases\n",cases);return 0;}
static int generator(void){for(unsigned n=0;n<=47;n++){CRCallerGenerator f;C(cr_caller_init(&f,n)==0);uint64_t a=0,b=1;for(unsigned i=0;i<n;i++){uint32_t out=91;C(cr_caller_next(&f,&out)==0&&out==a);uint64_t next=a+b;a=b;b=next;}uint32_t out=91;C(cr_caller_next(&f,&out)==CO_DONE&&out==91);C(cr_caller_next(&f,&out)==CO_DONE&&out==91);}return 0;}
typedef struct{unsigned ready,calls,used,key[64];uint32_t value[64];} Sink;
static int sink(void*v,unsigned key,uint32_t value){Sink*s=v;s->calls++;if(!s->ready)return CO_PENDING;s->key[s->used]=key;s->value[s->used++]=value;return CO_OK;}
static int push(void){const CORow rows[]={{1,2},{1,-1},{2,-1},{1,3},{1,4},{3,5}};
 for(size_t split=0;split<=6;split++){
  CFBatch b;C(cf_batch_open(&b,rows,6)==0);Sink s={0};size_t emitted=91;C(cr_push(&b,split,split==6,sink,&s,0,&emitted)==CO_PENDING&&emitted==0&&!s.calls);
  C(cr_push(&b,split,split==6,sink,&s,10,&emitted)==CO_PENDING&&emitted==0&&s.used==0&&b.committed==0);
  s.ready=1;int r=cr_push(&b,split,split==6,sink,&s,10,&emitted);C(r==CO_PENDING||r==CO_DONE);C(emitted==s.used);
  size_t old=s.used;r=cr_push(&b,6,1,sink,&s,10,&emitted);C(r==CO_DONE&&s.used==3&&emitted==3-old);
  C(s.key[0]==1&&s.value[0]==2&&s.key[1]==1&&s.value[1]==7&&s.key[2]==3&&s.value[2]==5);C(cf_batch_cancel(&b)==0);
 }return 0;}
static volatile uint64_t retained;
static int cost(void){
 for(unsigned sample=0;sample<12;sample++)for(unsigned alternate=0;alternate<2;alternate++){
  struct timespec a,b;C(clock_gettime(CLOCK_MONOTONIC,&a)==0);uint64_t sum=0;size_t pulls=0;
  for(unsigned repetition=0;repetition<1000;repetition++){
   COFlow f={{0,1000,1,0},200,0};CRUnfused u={.flow=f};uint32_t out=0;
   while((alternate?cr_unfused_next(&u,&out):co_flow_next(&f,&out))==0)sum+=out;
   pulls+=alternate?u.flow.pulls:f.pulls;
  }
  C(clock_gettime(CLOCK_MONOTONIC,&b)==0);retained=sum;
  printf("CX_COST,%u,%u,%lld,%zu,%llu,%zu\n",sample,alternate,(long long)(b.tv_sec-a.tv_sec)*1000000000LL+b.tv_nsec-a.tv_nsec,pulls,(unsigned long long)sum,alternate?sizeof(CRUnfused):sizeof(COFlow));
 }return 0;
}
int main(int argc,char**argv){if(argc!=2)return 2;if(!strcmp(argv[1],"iterator"))return iterator();if(!strcmp(argv[1],"generator"))return generator();if(!strcmp(argv[1],"push"))return push();if(!strcmp(argv[1],"cost"))return cost();return 2;}
