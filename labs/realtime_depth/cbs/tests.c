#include "cbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"CB_CHECK:%u:%s\n",__LINE__,#x);exit(1);}}while(0)
static void soft(void) {
  CBBudget s;CHECK(cb_init(&s,3,10,10,CB_SOFT_BOOK,0)==CMX_OK);CHECK(cb_arrive(&s,0)==CMX_OK);
  CHECK(s.core.remaining==3&&s.core.deadline==10);
  for(unsigned i=0;i<8;i++){CHECK(cb_charge(&s,i,1)==CMX_OK);CHECK(!s.throttled);
    if(i==2)CHECK(s.core.remaining==3&&s.core.deadline==20);
    if(i==5)CHECK(s.core.remaining==3&&s.core.deadline==30);}
  CHECK(s.core.remaining==1&&s.core.deadline==30);puts("CB_SOFT:immediate-refill-and-deadline-postponement");
}
static void equality(void) {
  CBBudget book,retained;
  CHECK(cb_init(&book,4,8,8,CB_SOFT_BOOK,0)==CMX_OK);CHECK(cb_init(&retained,4,8,8,CB_SOFT_RETAINED,0)==CMX_OK);
  CHECK(cb_arrive(&book,0)==CMX_OK&&cb_arrive(&retained,0)==CMX_OK);
  CHECK(cb_charge(&book,0,2)==CMX_OK&&cb_charge(&retained,0,2)==CMX_OK);
  CHECK(cb_idle(&book)==CMX_OK&&cb_idle(&retained)==CMX_OK);
  CHECK(cb_arrive(&book,4)==CMX_OK&&cb_arrive(&retained,4)==CMX_OK);
  CHECK(book.core.remaining==4&&book.core.deadline==12);
  CHECK(retained.core.remaining==2&&retained.core.deadline==8);
  puts("CB_EQUALITY:book-nonstrict-versus-retained-strict-boundary");
}
static void constrained(void) {
  CBBudget s;CHECK(cb_init(&s,2,4,10,CB_HARD_DOCUMENT,0)==CMX_OK);CHECK(cb_arrive(&s,0)==CMX_OK);
  CHECK(s.core.deadline==4);CHECK(cb_charge(&s,0,2)==CMX_OK);CHECK(s.throttled&&!s.core.remaining);
  CHECK(cb_update(&s,3)==CMX_OK&&s.throttled);CHECK(cb_update(&s,4)==CMX_OK&&!s.throttled);
  CHECK(s.core.deadline==14&&s.core.remaining==2);CHECK(cb_idle(&s)==CMX_OK);
  CHECK(cb_arrive(&s,12)==CMX_OK&&s.core.deadline==16);
  CHECK(cb_init(&s,2,4,10,CB_SOFT_BOOK,0)==CMX_INVALID);
  CHECK(cb_init(&s,2,4,10,CB_HARD_DOCUMENT,0)==CMX_OK&&cb_arrive(&s,0)==CMX_OK&&cb_charge(&s,0,2)==CMX_OK);
  CBBudget before=s;CHECK(cb_update(&s,14)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s));
  puts("CB_CONSTRAINED:documented-D-deadline-P-replenishment-comparison");
}
static void guards(void) {
  CBBudget s;CHECK(cb_init(&s,3,10,10,CB_SOFT_BOOK,0)==CMX_OK);CHECK(cb_arrive(&s,0)==CMX_OK);CBBudget before=s;
  CHECK(cb_charge(&s,1,4)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s));
  CHECK(cb_charge(&s,1,0)==CMX_INVALID);CHECK(!memcmp(&s,&before,sizeof s));
  CHECK(cb_update(&s,UINT32_MAX)==CMX_INVALID);CHECK(!memcmp(&s,&before,sizeof s));
  CHECK(cb_init(&s,2,6,6,CB_SOFT_BOOK,UINT32_MAX-3)==CMX_OK);CHECK(cb_arrive(&s,UINT32_MAX-3)==CMX_OK);
  CHECK(s.core.deadline==2);CHECK(cb_charge(&s,UINT32_MAX-3,2)==CMX_OK&&s.core.deadline==8);
  CHECK(cb_idle(&s)==CMX_OK&&cb_arrive(&s,1)==CMX_OK&&s.core.deadline==8);
  CHECK(cb_init(&s,1,0x7fffffff,0x7fffffff,CB_SOFT_BOOK,0)==CMX_OK);CHECK(cb_arrive(&s,0)==CMX_OK);before=s;
  CHECK(cb_charge(&s,0,1)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s));
  unsigned accepted=99;CHECK(cb_admit(1073741824,2147483647,2147483647,1073741824,2147483647,2147483647,&accepted)==CMX_OK&&!accepted);
  CHECK(cb_admit(1073741823,2147483647,2147483647,1073741823,2147483647,2147483647,&accepted)==CMX_OK&&accepted);
  CHECK(cb_admit(1,0,2,1,2,2,&accepted)==CMX_INVALID&&accepted==1);
  puts("CB_GUARDS:transaction-wrap-fullwidth-density-and-ordering-refusal");
}
static CBWorkload plan(void) {
  CBWorkload w={.quota={3,2},.deadline={10,5},.period={10,5},.policy={CB_SOFT_BOOK,CB_SOFT_BOOK},.count=1,.horizon=32,.capacity=8};
  w.request[0]=(CBRequest){0,0,8,10};return w;
}
static void workload(void) {
  CBWorkload w=plan();CBTrace trace;
  CHECK(cb_workload(&w,&trace)==CMX_OK&&trace.finish[0]==8);
  w.policy[0]=CB_HARD_DOCUMENT;CHECK(cb_workload(&w,&trace)==CMX_OK&&trace.finish[0]==22&&trace.missed[0]);
  w=plan();for(unsigned i=0;i<6;i++)w.request[w.count++]=(CBRequest){1,i*5,2,i*5+5};
  CHECK(cb_workload(&w,&trace)==CMX_OK&&trace.finish[0]==14);
  CHECK(trace.selected[4]==0&&trace.deadline[4][0]==20&&trace.selected[5]==1);
  for(unsigned i=1;i<w.count;i++)CHECK(trace.finish[i]==w.request[i].release+2&&!trace.missed[i]);
  puts("CB_WORKLOAD:soft8-throttle22-competitor14-and-application-deadlines");
}
static void mutex_case(void) {
  CBMutexTrace t;CHECK(cb_mutex(2,&t)==CMX_OK&&t.waiter_finish==3&&!t.waiter_missed);
  CHECK(cb_mutex(7,&t)==CMX_OK&&t.waiter_finish==8&&t.waiter_missed);
  for(unsigned i=1;i<7;i++)CHECK(t.waiter_blocked[i]&&t.owner[i]==0&&t.charged[i]==0&&t.budget[i][1]==2);
  CHECK(t.owner[7]==1&&t.charged[7]==1&&t.budget[7][1]==1);
  puts("CB_MUTEX:actual-CMX-blocking-breaks-independent-reservation-premise");
}
static void stream(void) {
  unsigned kind0;
  while(scanf("%u",&kind0)==1) {
    CBWorkload w={0};unsigned kind1;
    CHECK(scanf("%u %u %u %u %u %u %u %u %u %u",&w.quota[0],&w.deadline[0],&w.period[0],&kind1,
      &w.quota[1],&w.deadline[1],&w.period[1],&w.capacity,&w.horizon,&w.count)==10);
    CHECK(w.count<=CB_REQUESTS);w.policy[0]=(enum cb_policy)kind0;w.policy[1]=(enum cb_policy)kind1;
    for(unsigned i=0;i<w.count;i++)CHECK(scanf("%u %u %u %u",&w.request[i].client,&w.request[i].release,&w.request[i].cost,&w.request[i].application_deadline)==4);
    CBTrace t,before;memset(&t,0xa5,sizeof t);before=t;
    enum cmx_result result=cb_workload(&w,&t);
    if(result!=CMX_OK){CHECK(result==CMX_FULL&&!memcmp(&t,&before,sizeof t));printf("E %u\n",(unsigned)result);continue;}
    for(unsigned i=0;i<w.horizon;i++)printf("T %u %d %u %u %u %u %u %u\n",i,t.selected[i],t.budget[i][0],t.budget[i][1],t.deadline[i][0],t.deadline[i][1],t.pending[i][0],t.pending[i][1]);
    for(unsigned i=0;i<w.count;i++)printf("R %u %u %u %u %u %u %u\n",i,t.admitted[i],t.rejected[i],t.missed[i],t.finish[i],t.executed[i],t.remaining[i]);
  }
}
int main(int argc,char **argv){CHECK(argc==2);
  if(!strcmp(argv[1],"soft"))soft();
  else if(!strcmp(argv[1],"equality"))equality();
  else if(!strcmp(argv[1],"constrained"))constrained();
  else if(!strcmp(argv[1],"guards"))guards();
  else if(!strcmp(argv[1],"workload"))workload();
  else if(!strcmp(argv[1],"mutex"))mutex_case();
  else if(!strcmp(argv[1],"stream"))stream();
  else CHECK(0);
  return 0;
}
