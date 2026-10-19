#include "response.h"
#include "cmx.h"
#ifdef RTA_TARGET
#define CHECK(x) do{if(!(x))rta_target_stop(1);}while(0)
#define SAY(x) rta_target_puts(x)
#else
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RTA_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
#define SAY(x) puts(x)
#endif
static int same_result(const RTAResult *a,const RTAResult *b){
  if(a->estimate!=b->estimate||a->iterations!=b->iterations)return 0;
  for(unsigned i=0;i<=RTA_STEPS;i++)if(a->trace[i]!=b->trace[i])return 0;
  return 1;
}
static int basic(void){
  RTATask tasks[2]={{.cost=1,.period=4,.deadline=4},{.cost=2,.period=10,.deadline=10,.blocking=1}};RTAResult r;
  CHECK(rta_response(tasks,2,1,8,&r)==RTA_CONVERGED&&r.estimate==4&&r.iterations==2&&r.trace[0]==3&&r.trace[1]==4&&r.trace[2]==4);
  cmx_job old[2]={{1,4,4,0,0},{2,10,10,0,1}};uint32_t original=0;
  CHECK(cmx_response(old,2,1,&original)==CMX_OK&&original==r.estimate);
  tasks[1].deadline=4;CHECK(rta_response(tasks,2,1,8,&r)==RTA_CONVERGED&&r.estimate==4);
  tasks[1].deadline=3;CHECK(rta_response(tasks,2,1,8,&r)==RTA_EXCEEDED&&r.estimate==4&&r.iterations==1);
  tasks[1].cost=0;tasks[1].blocking=0;tasks[0].jitter=9;CHECK(rta_response(tasks,2,1,8,&r)==RTA_CONVERGED&&r.estimate==0&&r.iterations==0);
  SAY("RTA_BASIC:3,4,4;actual-CMX-comparison;exact-deadline;first-exceeded;zero-work");return 0;
}
static int jitter(void){
  RTATask tasks[2]={{.cost=1,.period=4,.deadline=4,.jitter=1},{.cost=2,.period=10,.deadline=10,.blocking=1}};RTAResult r;
  CHECK(rta_response(tasks,2,1,8,&r)==RTA_CONVERGED&&r.estimate==5&&r.iterations==3&&r.trace[0]==3&&r.trace[1]==4&&r.trace[2]==5&&r.trace[3]==5);
  tasks[1].deadline=4;CHECK(rta_response(tasks,2,1,8,&r)==RTA_EXCEEDED&&r.estimate==5&&r.iterations==2);
  SAY("RTA_JITTER:3,4,5,5;deadline4-first-exceeded5;arrival-count-bound");return 0;
}
static int arithmetic(void){
  uint64_t q=777;
  CHECK(rta_ceil_sum(UINT64_MAX,UINT64_MAX,UINT64_MAX,&q)==RTA_CONVERGED&&q==2);
  CHECK(rta_ceil_sum(UINT64_MAX-1,0,UINT64_MAX,&q)==RTA_CONVERGED&&q==1);
  CHECK(rta_ceil_sum(UINT64_MAX,0,1,&q)==RTA_CONVERGED&&q==UINT64_MAX);q=777;
  CHECK(rta_ceil_sum(UINT64_MAX,1,1,&q)==RTA_OVERFLOW&&q==777);
  CHECK(rta_ceil_sum(1,0,0,&q)==RTA_INVALID&&q==777);
  RTATask t[2]={{.cost=1,.period=UINT64_MAX,.deadline=UINT64_MAX,.jitter=UINT64_MAX-4},
    {.cost=UINT64_MAX-3,.period=UINT64_MAX,.deadline=UINT64_MAX}};RTAResult r={.estimate=777};
  CHECK(rta_response(t,2,1,8,&r)==RTA_CONVERGED&&r.estimate==UINT64_MAX-1);
  t[1].cost=UINT64_MAX;t[1].blocking=1;r.estimate=777;r.iterations=42;
  for(unsigned i=0;i<=RTA_STEPS;i++)r.trace[i]=100+i;
  RTAResult unchanged=r;
  CHECK(rta_response(t,2,1,8,&r)==RTA_OVERFLOW&&same_result(&r,&unchanged));
  t[1].cost=1;t[1].blocking=0;t[0].cost=UINT64_MAX/2+1;t[0].period=1;t[0].jitter=0;
  CHECK(rta_response(t,2,1,8,&r)==RTA_OVERFLOW&&same_result(&r,&unchanged));
  t[0].period=0;CHECK(rta_response(t,2,1,8,&r)==RTA_INVALID&&same_result(&r,&unchanged));
  CHECK(rta_response(t,2,2,8,&r)==RTA_INVALID&&same_result(&r,&unchanged));
  CHECK(rta_response(t,0,0,8,&r)==RTA_INVALID&&same_result(&r,&unchanged));
  CHECK(rta_response(t,RTA_TASKS+1,0,8,&r)==RTA_INVALID&&same_result(&r,&unchanged));
  SAY("RTA_ARITHMETIC:full-u64;65-bit-numerator;checked-cost-product;transactional-refusal");return 0;
}
static int limit(void){
  RTATask t[2]={{.cost=1,.period=1,.deadline=1},{.cost=1,.period=100,.deadline=100}};RTAResult r={.estimate=777};
  CHECK(rta_response(t,2,1,3,&r)==RTA_LIMIT&&r.estimate==4&&r.iterations==3&&r.trace[0]==1&&r.trace[1]==2&&r.trace[2]==3&&r.trace[3]==4);
  r.estimate=777;CHECK(rta_response(t,2,1,0,&r)==RTA_INVALID&&r.estimate==777);
  CHECK(rta_response(t,2,1,RTA_STEPS+1,&r)==RTA_INVALID&&r.estimate==777);
  SAY("RTA_LIMIT:1,2,3,4;resource-exhaustion-is-not-convergence;unchanged-invalid-output");return 0;
}
static int interference(void){
  RTATask t[3]={{.cost=1,.period=4,.deadline=4,.overhead=1},{.cost=2,.period=10,.deadline=10,.blocking=1,.overhead=1}};RTAResult r;
  CHECK(rta_response(t,2,1,16,&r)==RTA_CONVERGED&&r.estimate==8&&r.trace[0]==4&&r.trace[1]==6&&r.trace[2]==8&&r.trace[3]==8);
  t[0]=(RTATask){.cost=1,.period=3,.deadline=3};t[1]=(RTATask){.cost=1,.period=4,.deadline=4};
  t[2]=(RTATask){.cost=2,.period=10,.deadline=10,.blocking=1};
  CHECK(rta_response(t,3,2,16,&r)==RTA_CONVERGED&&r.estimate==8&&r.trace[0]==3&&r.trace[1]==5&&r.trace[2]==7&&r.trace[3]==8&&r.trace[4]==8);
  SAY("RTA_INTERFERENCE:per-job-overhead-once;IRQ-separate-high-priority-arrival;no-double-counting");return 0;
}
#ifndef RTA_TARGET
static int stream(void){
  unsigned kind;
  while(scanf("%u",&kind)==1){
    if(kind==1){uint64_t a,b,d,q=777;if(scanf("%"SCNu64" %"SCNu64" %"SCNu64,&a,&b,&d)!=3)return 2;
      enum rta_status status=rta_ceil_sum(a,b,d,&q);printf("A %u %"PRIu64"\n",(unsigned)status,q);
    }else if(kind==2){unsigned n,target,cap;if(scanf("%u %u %u",&n,&target,&cap)!=3||n>RTA_TASKS)return 2;
      RTATask tasks[RTA_TASKS];for(unsigned i=0;i<n;i++)if(scanf("%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64,
        &tasks[i].cost,&tasks[i].period,&tasks[i].deadline,&tasks[i].jitter,&tasks[i].blocking,&tasks[i].overhead)!=6)return 2;
      RTAResult r={.estimate=777};enum rta_status status=rta_response(tasks,n,target,cap,&r);
      printf("R %u %"PRIu64" %u",(unsigned)status,r.estimate,r.iterations);for(unsigned i=0;i<=r.iterations;i++)printf(" %"PRIu64,r.trace[i]);puts("");
    }else return 2;
  }
  return 0;
}
int main(int argc,char **argv){
  if(argc!=2)return 2;
  if(!strcmp(argv[1],"basic"))return basic();
  if(!strcmp(argv[1],"jitter"))return jitter();
  if(!strcmp(argv[1],"arithmetic"))return arithmetic();
  if(!strcmp(argv[1],"limit"))return limit();
  if(!strcmp(argv[1],"interference"))return interference();
  if(!strcmp(argv[1],"stream"))return stream();
  return 2;
}
#endif
