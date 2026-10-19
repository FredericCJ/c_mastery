#include "jump_scope.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ER_CHECK EJD line%d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct Stack Stack;
typedef struct {Stack *stack;unsigned digit;} Entry;
struct Stack {er_frame frame;unsigned wanted,cut,calls,attempts,trace[4];er_code code;Entry entry[4];};
static void append(void *p){Entry *e=p;Stack *s=e->stack;if(s->calls<4)s->trace[s->calls++]=e->digit;}
static er_result stack_body(er_frame *frame,void *p){
 Stack *s=p;
 if(s->code&&s->cut==0){(void)er_raise(frame,s->code);return er_error(ER_INVALID,0);}
 for(unsigned i=0;i<s->wanted;++i){
  ++s->attempts;er_code code=er_defer(frame,append,&s->entry[i<4?i:3]);if(code!=ER_OK)return er_error(code,i);
  if(s->code&&s->cut==i+1){(void)er_raise(frame,s->code);return er_error(ER_INVALID,0);}
 }
 return er_ok(17);
}
static er_result stack_run(Stack *s){for(unsigned i=0;i<4;++i)s->entry[i]=(Entry){s,i+1};return er_run(&s->frame,stack_body,s);}
static int stack(void){
 for(unsigned n=0;n<=5;++n)for(unsigned cut=0;cut<=6;++cut)for(unsigned code=0;code<=8;++code){
  Stack s={0};s.wanted=n;s.cut=cut;s.code=(er_code)code;er_result r=stack_run(&s);unsigned registered=n<4?n:4,attempts=n<5?n:5;
  er_code expected=n>4?ER_RANGE:ER_OK;size_t value=n>4?4:17;
  if(code&&cut<=registered){registered=cut;attempts=cut;expected=(er_code)code;value=0;}
  CHECK(r.tag==expected&&(r.tag==ER_OK?r.as.value:r.as.offset)==value&&s.calls==registered&&s.attempts==attempts);
  for(unsigned i=0;i<registered;++i)CHECK(s.trace[i]==registered-i);
  CHECK(s.frame.count==0&&!s.frame.active&&!s.frame.cleaning&&er_raise(&s.frame,ER_IO)==ER_INVALID&&er_defer(&s.frame,append,&s.entry[0])==ER_INVALID);
 }
 puts("EJD_STACK actual-er-boundary every-code registration-cuts capacity-four LIFO inactive-refusal");return 0;
}
static int resources(void){
 for(unsigned cut=5;cut<=7;++cut){
  OwAllocator a={0};unsigned effects=0;EjdReport r={0};EjdPlan p={cut,cut==7?ER_OK:ER_THROWN};CHECK(ejd_run(ow_allocator(&a),p,&effects,&r)==EJD_OK);
  CHECK(a.attempts==5&&a.live==0&&a.releases==5&&r.held==0&&effects==3&&r.scope_finishes==1&&r.scope_empty&&r.frames_inactive&&r.outer_live);
  unsigned order[5]={4,3,2,5,1},remaining[5]={14,6,2,2,0};CHECK(r.trace_count==5);for(unsigned i=0;i<5;++i)CHECK(r.trace[i]==order[i]&&r.remaining[i]==remaining[i]);
  CHECK(r.result.tag==(cut==7?ER_OK:ER_CONTRACT));
 }
 OwAllocator a={0};a.fail_at=4;unsigned effects=0;EjdReport r={0};CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_OK},&effects,&r)==EJD_OK);
 CHECK(a.attempts==5&&a.live==0&&a.releases==4&&r.inner.tag==ER_IO&&r.stage==10&&r.result.tag==ER_CONTRACT&&r.result.as.offset==10&&effects==2);
 puts("EJD_RESOURCES actual-T66-scope third-acquire-failure after-third-acquisition registered-unregistered-cleanup");return 0;
}
static int nested(void){
 OwAllocator a={0};unsigned effects=0;EjdReport r={0};CHECK(ejd_run(ow_allocator(&a),(EjdPlan){3,ER_IO},&effects,&r)==EJD_OK);
 CHECK(r.inner.tag==ER_IO&&r.result.tag==ER_CONTRACT&&r.result.as.offset==3&&r.stage==3&&r.trace_count==4&&r.outer_live);
 CHECK(r.trace[0]==3&&r.trace[1]==2&&r.trace[2]==5&&r.trace[3]==1&&r.remaining[1]==2&&effects==2&&r.scope_finishes==1&&r.scope_empty&&r.frames_inactive&&a.live==0);
 puts("EJD_NESTED separate-live-frames inner-cause retained-stage outer-order-preserved");return 0;
}
static int effects(void){
 OwAllocator a={0};unsigned published=UINT_MAX-3;EjdReport r={0};CHECK(ejd_run(ow_allocator(&a),(EjdPlan){5,ER_THROWN},&published,&r)==EJD_OK);
 CHECK(published==UINT_MAX&&r.effects_after==UINT_MAX&&a.live==0&&a.releases==5&&r.result.tag==ER_CONTRACT);
 a=(OwAllocator){0};published=0;CHECK(ejd_run(ow_allocator(&a),(EjdPlan){1,ER_IO},&published,&r)==EJD_OK);
 CHECK(published==1&&a.live==0&&a.releases==3&&r.result.tag==ER_CONTRACT);
 puts("EJD_EFFECTS published-counter-not-rollback actual-typed-allocation-drained finite-headroom");return 0;
}
static int admission(void){
 OwAllocator a={0};unsigned effects=UINT_MAX-2;EjdReport r={0};r.stage=99;
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_OK},&effects,&r)==EJD_INVALID&&effects==UINT_MAX-2&&r.stage==99&&a.attempts==0);
 effects=0;CHECK(ejd_run(ow_allocator(&a),(EjdPlan){8,ER_OK},&effects,&r)==EJD_INVALID&&r.stage==99);
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){1,ER_OK},&effects,&r)==EJD_INVALID&&r.stage==99);
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_IO},&effects,&r)==EJD_INVALID&&r.stage==99);
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_OK},NULL,&r)==EJD_INVALID&&r.stage==99);
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_OK},&effects,NULL)==EJD_INVALID&&a.attempts==0);
 CHECK(ejd_run((V4Allocator){0},(EjdPlan){7,ER_OK},&effects,&r)==EJD_INVALID&&a.attempts==0);
 CHECK(ejd_run(ow_allocator(&a),(EjdPlan){7,ER_OK},&effects,&r)==EJD_OK&&r.result.tag==ER_OK&&r.result.as.value==99);
 puts("EJD_ADMISSION malformed-plan null finite-effects no-alloc-or-output-before-admission");return 0;
}
static int oracle(void){
 unsigned fail,cut,code,initial;
 while(scanf("%u %u %u %u",&fail,&cut,&code,&initial)==4){OwAllocator a={0};a.fail_at=fail;unsigned effects=initial;EjdReport r={0};r.stage=99;int result=ejd_run(ow_allocator(&a),(EjdPlan){cut,(er_code)code},&effects,&r);
  printf("%d %u %zu %zu %zu %d %zu %d %zu %u %u %u %u %u %u %u %u",result,effects,a.attempts,a.releases,a.live,r.result.tag,r.result.tag==ER_OK?r.result.as.value:r.result.as.offset,r.inner.tag,r.inner.tag==ER_OK?r.inner.as.value:r.inner.as.offset,r.stage,r.held,r.scope_finishes,r.scope_empty,r.frames_inactive,r.inner_entered,r.outer_live,r.trace_count);
  for(unsigned i=0;i<5;++i)printf(" %u %u",r.trace[i],r.remaining[i]);
  putchar('\n');
 }
 return ferror(stdin)?2:0;
}
static int metrics(void){
 double samples[7];size_t attempts=0,releases=0;unsigned long checksum=0;
 for(unsigned sample=0;sample<7;++sample){clock_t start=clock();for(unsigned i=0;i<1000;++i){OwAllocator a={0};unsigned effects=0;EjdReport r={0};CHECK(ejd_run(ow_allocator(&a),(EjdPlan){5,ER_THROWN},&effects,&r)==EJD_OK&&r.result.tag==ER_CONTRACT&&a.live==0);checksum+=r.stage+effects+r.trace_count;attempts+=a.attempts;releases+=a.releases;}samples[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}
 printf("{\"iterations\":1000,\"checksum\":%lu,\"allocation_calls\":%zu,\"release_calls\":%zu,\"frame_size\":%zu,\"scope_size\":%zu,\"report_size\":%zu,\"seconds\":[",checksum,attempts,releases,sizeof(er_frame),sizeof(OwScope),sizeof(EjdReport));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",samples[i]);puts("]}");return 0;
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"jump_stack"))return stack();
 if(!strcmp(argv[1],"jump_resources"))return resources();
 if(!strcmp(argv[1],"jump_nested"))return nested();
 if(!strcmp(argv[1],"jump_effects"))return effects();
 if(!strcmp(argv[1],"jump_admission"))return admission();
 if(!strcmp(argv[1],"oracle"))return oracle();
 if(!strcmp(argv[1],"metrics"))return metrics();
 return 2;
}
