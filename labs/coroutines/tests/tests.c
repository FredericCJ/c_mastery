#include "coroutines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CR_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int stop_after(void *ctx,uint32_t x){unsigned *n=ctx;(void)x;return ++*n==2;}
static int iterator(void){
 struct co_flow f={{0,10,1,0},3,0};uint32_t x=91;CHECK(co_flow_next(&f,&x)==0&&x==0&&f.pulls==1);
 CHECK(co_flow_next(&f,&x)==0&&x==4&&f.pulls==3);CHECK(co_flow_next(&f,&x)==0&&x==16&&f.pulls==5);
 CHECK(co_flow_next(&f,&x)==CO_DONE&&x==16&&f.pulls==5);
 f=(struct co_flow){{0,10,1,0},10,0};unsigned calls=0;size_t accepted=91;
 CHECK(co_fold(&f,stop_after,&calls,&accepted)==0&&calls==2&&accepted==2&&f.pulls==3);
 CHECK(co_flow_next(&f,&x)==0&&x==16);return 0;
}
static int dataflow(void){
 struct co_row rows[]={{1,-1},{1,3},{1,4},{2,-8},{3,5},{4,6}};struct co_query q;unsigned k=91;uint32_t v=92;
 CHECK(co_query_open(&q,rows,6)==0);CHECK(co_query_next(&q,&k,&v)==0&&k==1&&v==7&&q.index==3);
 CHECK(co_query_next(&q,&k,&v)==0&&k==3&&v==5&&q.index==5);
 CHECK(co_query_next(&q,&k,&v)==0&&k==4&&v==6);
 CHECK(co_query_next(&q,&k,&v)==CO_DONE&&k==4&&v==6);
 CHECK(co_query_close(&q)==0&&co_query_close(&q)==0&&co_query_next(&q,&k,&v)==CO_STATE);
 struct co_row overflow[]={{1,INT32_MAX},{1,INT32_MAX},{1,2}};CHECK(co_query_open(&q,overflow,3)==0);
 CHECK(co_query_next(&q,&k,&v)==CO_RANGE&&q.index==0&&v==6);return 0;
}
struct co_alloc{unsigned allocations,releases,fail;void *last;};
static void *alloc(void *ctx,size_t n){struct co_alloc *a=ctx;a->allocations++;if(a->fail)return NULL;a->last=malloc(n);return a->last;}
static void release(void *ctx,void *p){struct co_alloc *a=ctx;a->releases++;free(p);a->last=NULL;}
static int generator(void){
 struct co_alloc a={0};struct co_allocator port={&a,alloc,release};struct co_generator *g=NULL;uint32_t x=91;
 a.fail=1;CHECK(co_generator_create(port,3,&g)==CO_RANGE&&!g&&a.releases==0);a.fail=0;
 CHECK(co_generator_create(port,3,&g)==0&&g);CHECK(co_generator_next(g,&x)==0&&x==0);
 CHECK(co_generator_next(g,&x)==0&&x==1);CHECK(co_generator_next(g,&x)==0&&x==1);
 CHECK(co_generator_next(g,&x)==CO_DONE&&x==1);CHECK(co_generator_destroy(&g)==0&&!g);
 /* Free a deliberately retained mutant allocation only in the host harness;
  * this keeps leak detection from replacing the semantic assertion. */
 if(a.last){free(a.last);a.last=NULL;}
 CHECK(a.releases==1);CHECK(co_generator_destroy(&g)==0&&a.releases==1);
 CHECK(co_generator_create(port,47,&g)==0);for(unsigned i=0;i<47;i++)CHECK(co_generator_next(g,&x)==0);
 CHECK(x==1836311903u&&co_generator_next(g,&x)==CO_DONE);CHECK(co_generator_destroy(&g)==0&&a.releases==2);return 0;
}
static int cps(void){
 struct co_trampoline p;uint32_t out=91;CHECK(co_trampoline_init(&p,6)==0);
 CHECK(co_trampoline_step(&p,0,&out)==CO_PENDING&&p.n==6&&out==91);
 for(unsigned i=0;i<13;i++)CHECK(co_trampoline_step(&p,1,&out)==CO_PENDING&&out==91);
 CHECK(co_trampoline_step(&p,1,&out)==CO_DONE&&out==720&&p.depth==0);
 CHECK(co_trampoline_step(&p,1,&out)==CO_DONE&&out==720);
 CHECK(co_trampoline_init(&p,12)==0&&co_trampoline_step(&p,5,&out)==CO_PENDING);
 CHECK(co_trampoline_cancel(&p)==0&&p.depth==0&&co_trampoline_step(&p,1,&out)==CO_STATE);return 0;
}
static int parser(void){
 struct co_parser p[]={{CO_CHOICE,1,4,0},{CO_SEQUENCE,2,3,0},{CO_LITERAL,0,0,'a'},{CO_LITERAL,0,0,'b'},{CO_LITERAL,0,0,'a'}};
 size_t end=91;unsigned steps=0;CHECK(co_parse(p,5,(const unsigned char*)"ac",2,0,20,&end,&steps)==0&&end==1);
 CHECK(co_parse(p,5,(const unsigned char*)"ab",2,0,20,&end,&steps)==0&&end==2);
 end=91;CHECK(co_parse(p,5,(const unsigned char*)"x",1,0,20,&end,&steps)==CO_NO_MATCH&&end==91);
 CHECK(co_parse(p,5,NULL,0,0,0,&end,&steps)==CO_RANGE&&end==91&&steps==0);
 struct co_parser stalled[]={{CO_MANY,1,0,0},{CO_EMPTY,0,0,0}};
 CHECK(co_parse(stalled,2,NULL,0,0,20,&end,&steps)==CO_STATE&&end==91);return 0;
}
static int packrat(void){
 struct co_memo memo[65];size_t end=91;unsigned calls=0;
 CHECK(co_packrat((const unsigned char*)"aaxcc",5,memo,65,&end,&calls)==0&&end==5&&calls==3);
 unsigned char s[25];for(unsigned i=0;i<12;i++)s[i]='a';s[12]='x';for(unsigned i=13;i<25;i++)s[i]='c';
 CHECK(co_packrat(s,25,memo,65,&end,&calls)==0&&end==25&&calls==13);
 end=91;CHECK(co_packrat((const unsigned char*)"aac",3,memo,65,&end,&calls)==CO_NO_MATCH&&end==91&&calls<=4);
 CHECK(co_packrat(s,25,memo,25,&end,&calls)==CO_INVALID&&end==91);return 0;
}
struct async_port{unsigned reads,arms,disarms,mode;uint32_t generation;};
static struct co_read_result read_fake(void *ctx,unsigned char *p,size_t n){struct async_port *a=ctx;a->reads++;if(a->mode==1||!a->arms)return(struct co_read_result){CO_READ_AGAIN,0};for(size_t i=0;i<n;i++)p[i]=(unsigned char)(10+i);return(struct co_read_result){CO_READ_DATA,n};}
static int arm(void *ctx,uint32_t g){struct async_port *a=ctx;a->arms++;a->generation=g;return 0;}
static void unarm(void *ctx,uint32_t g){struct async_port *a=ctx;if(g==a->generation)a->disarms++;}
static int async(void){
 struct async_port a={0};struct co_async_port port={&a,read_fake,arm,unarm};struct co_async f;unsigned char out[8]={91};size_t n=92;
 CHECK(co_async_init(&f,3,7)==0);CHECK(co_async_poll(&f,port,out,&n)==CO_DONE&&n==3&&out[0]==10&&a.reads==2&&a.arms==1&&a.disarms==1);
 CHECK(co_async_poll(&f,port,out,&n)==CO_DONE&&a.reads==2);
 a=(struct async_port){0};a.mode=1;CHECK(co_async_init(&f,3,8)==0&&co_async_poll(&f,port,out,&n)==CO_PENDING);
 CHECK(co_async_notify(&f,7)==CO_STATE&&co_async_notify(&f,8)==0);
 CHECK(co_async_cancel(&f,port)==0&&a.disarms==1&&!f.registered);
 unsigned old=a.reads;CHECK(co_async_poll(&f,port,out,&n)==CO_STATE&&a.reads==old);return 0;
}
static int effect(void){
 struct co_effect p;struct co_continuation a,b,stale;uint32_t out=91;
 CHECK(co_effect_start(&p,&a)==CO_REQUEST);stale=a;
 CHECK(co_effect_resume(&a,12,&b,&out)==CO_REQUEST&&!a.owner&&b.question==2&&out==91);
 CHECK(co_effect_resume(&stale,1,&a,&out)==CO_STATE&&out==91);
 CHECK(co_effect_resume(&b,30,&a,&out)==CO_DONE&&!b.owner&&out==42);
 CHECK(co_effect_start(&p,&a)==CO_REQUEST&&co_effect_abort(&a)==0&&!a.owner&&p.pc==4);return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;
#define RUN(x) if(!strcmp(argv[1],#x))return x()
 RUN(iterator);RUN(dataflow);RUN(generator);RUN(cps);RUN(parser);RUN(packrat);RUN(async);RUN(effect);return 2;}
