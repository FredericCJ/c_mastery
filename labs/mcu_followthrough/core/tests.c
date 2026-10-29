#include "mcu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"MF_CHECK:%s:%d:%s\n",__func__,__LINE__,#x); return 1; } } while (0)
static _Alignas(8) uint32_t stacks[CMX_TASKS+2][160];
static int initialize(struct mf_mf *m) { return mf_init(m,stacks[0],160,CMX_RR)!=CMX_OK; }
static mf_handle_id add(struct mf_mf *m,unsigned slot,unsigned detached) {
  mf_handle_id h=0;
  if(mf_add(m,stacks[slot],160,0x101,0x201,4,detached,2,&h)!=CMX_OK) return 0;
  return h;
}
static int admission(void) {
  struct mf_mf m,before; CHECK(!initialize(&m)); CHECK(mf_invariant(&m));
  mf_handle_id h=999; memcpy(&before,&m,sizeof m);
  CHECK(mf_add(&m,stacks[1],127,0x101,0x201,4,0,2,&h)==CMX_INVALID);
  CHECK(h==999 && !memcmp(&m,&before,sizeof m));
  CHECK(mf_add(&m,stacks[1]+1,159,0x101,0x201,4,0,2,&h)==CMX_INVALID);
  CHECK(mf_add(&m,stacks[0],160,0x101,0x201,4,0,2,&h)==CMX_INVALID);
  CHECK(mf_add(&m,stacks[1],160,0x100,0x201,4,0,2,&h)==CMX_INVALID);
  for(unsigned budget=0;budget<2;++budget) {
    uint32_t sentinel=stacks[1][0];
    CHECK(mf_add(&m,stacks[1],160,0x101,0x201,4,0,budget,&h)==CMX_FULL);
    CHECK(h==999 && !memcmp(&m,&before,sizeof m) && stacks[1][0]==sentinel);
  }
  CHECK(mf_add(&m,stacks[1],129,0x101,0x201,4,1,2,&h)==CMX_OK);
  CHECK(mf_lookup(&m,h)==1 && m.kernel.task[1].stack_high==(uintptr_t)(stacks[1]+128));
  uint32_t *frame=m.sp[1]; CHECK(frame==stacks[1]+128-17);
  for(unsigned i=0;i<8;++i) CHECK(frame[i]==0);
  CHECK(frame[8]==0xfffffffd && frame[14]==0x201 && frame[15]==0x101 && frame[16]==0x01000000);
  CHECK((m.kernel.task[1].stack_high&7u)==0 && ((uintptr_t)frame&7u)==4);
  for(unsigned i=2;i<CMX_TASKS;++i) CHECK(add(&m,i,1)!=0);
  CHECK(mf_invariant(&m)); memcpy(&before,&m,sizeof m); h=999;
  CHECK(mf_add(&m,stacks[16],160,0x101,0x201,4,1,2,&h)==CMX_FULL);
  CHECK(!memcmp(&m,&before,sizeof m) && h==999 && m.kernel.task[0].state==CMX_READY);
  CHECK(mf_lookup(&m,0)==-1 && mf_lookup(&m,UINT64_MAX)==-1);
  return 0;
}
static int lifecycle(void) {
  struct mf_mf m; CHECK(!initialize(&m)); mf_handle_id a=add(&m,1,0),b=add(&m,2,0),c=add(&m,3,0); CHECK(a&&b&&c);
  int value=999; CHECK(mf_join(&m,a,a,cmx_forever(),&value)==CMX_INVALID);
  CHECK(mf_join(&m,a,b,cmx_forever(),&value)==CMX_BLOCKED && value==999);
  CHECK(mf_join(&m,c,b,cmx_forever(),&value)==CMX_INVALID);
  CHECK(cmx_cancel(&m.kernel,1)==CMX_OK && m.kernel.task[2].joiner==-1);
  CHECK(mf_join(&m,c,b,cmx_forever(),&value)==CMX_BLOCKED);
  CHECK(mf_exit(&m,b,73)==CMX_OK);
  CHECK(mf_join_result(&m,c,b,&value)==CMX_OK && value==73);
  CHECK(mf_join_result(&m,c,b,&value)==CMX_INVALID);
  m.executing=2; CHECK(mf_reap(&m,b)==CMX_INVALID); m.executing=3;
  CHECK(mf_reap(&m,b)==CMX_OK && mf_lookup(&m,b)==-1);
  mf_handle_id newer=add(&m,2,0); CHECK(newer && newer!=b);
  CHECK(mf_exit(&m,newer,91)==CMX_OK);
  CHECK(mf_join(&m,a,newer,cmx_forever(),&value)==CMX_OK && value==91);
  CHECK(mf_join(&m,a,newer,cmx_forever(),&value)==CMX_INVALID);
  CHECK(mf_reap(&m,newer)==CMX_OK);
  newer=add(&m,2,1); CHECK(newer);
  CHECK(mf_join(&m,a,newer,cmx_forever(),&value)==CMX_INVALID);
  CHECK(mf_exit(&m,newer,82)==CMX_OK && mf_reap(&m,newer)==CMX_OK);
  /* The selected slot's generation is saturated without wrapping/reusing. */
  m.kernel.task[2].generation=UINT32_MAX; newer=add(&m,2,1); CHECK(newer);
  CHECK(mf_exit(&m,newer,0)==CMX_OK && mf_reap(&m,newer)==CMX_FULL);
  CHECK(m.kernel.task[2].state==CMX_EXITED && m.kernel.task[2].generation==UINT32_MAX);
  CHECK(mf_invariant(&m));
  return 0;
}
static int scheduling(void) {
  struct mf_mf m; CHECK(!initialize(&m)); CHECK(mf_schedule(&m)==0);
  mf_handle_id a=add(&m,1,1),b=add(&m,2,1); CHECK(a&&b);
  for(unsigned i=0;i<20;++i) { CHECK(mf_schedule(&m)==(int)(1+i%2)); CHECK(mf_invariant(&m)); }
  CHECK(cmx_sleep(&m.kernel,1,cmx_at(5))==CMX_BLOCKED);
  for(unsigned i=0;i<4;++i) CHECK(mf_schedule(&m)==2);
  CHECK(mf_exit(&m,b,0)==CMX_OK && mf_schedule(&m)==0);
  cmx_tick(&m.kernel,5); CHECK(mf_schedule(&m)==1);
  m.kernel.policy=CMX_FIXED; mf_handle_id c=add(&m,3,1); CHECK(c);
  m.kernel.task[3].base_priority=m.kernel.task[3].priority=2;
  CHECK(mf_schedule(&m)==3); CHECK(mf_invariant(&m));
  /* Every serialized ordering of give/cancel/timeout has exactly one result. */
  unsigned permutations[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
  for(unsigned p=0;p<6;++p) {
    CHECK(!initialize(&m)); a=add(&m,1,1); CHECK(a);
    cmx_sem sem; cmx_sem_init(&sem,0,1);
    CHECK(cmx_sem_take(&m.kernel,&sem,1,cmx_at(10))==CMX_BLOCKED);
    enum cmx_result first=CMX_UNIMPLEMENTED;
    for(unsigned i=0;i<3;++i) {
      unsigned action=permutations[p][i];
      if(!action) (void)cmx_sem_give(&m.kernel,&sem);
      else if(action==1) (void)cmx_cancel(&m.kernel,1);
      else cmx_tick(&m.kernel,10);
      if(!i) first=m.kernel.task[1].result;
      CHECK(m.kernel.task[1].result==first && m.kernel.task[1].state==CMX_READY && m.kernel.task[1].wait==CMX_W_NONE);
      CHECK(mf_invariant(&m));
    }
    CHECK(first==(permutations[p][0]==0?CMX_OK:permutations[p][0]==1?CMX_CLOSED:CMX_TIMEOUT));
  }
  return 0;
}
static int faults(void) {
  struct mf_mf m; CHECK(!initialize(&m)); mf_handle_id a=add(&m,1,1),b=add(&m,2,1); CHECK(a&&b);
  CHECK(cmx_sleep(&m.kernel,1,cmx_forever())==CMX_BLOCKED);
  CHECK(mf_fault(&m,a,6,0x123)==CMX_OK && m.kernel.task[1].state==CMX_EXITED);
  CHECK(mf_fault(&m,a,7,0x456)==CMX_INVALID && m.fault_code[1]==6 && m.fault_pc[1]==0x123);
  CHECK(!m.fatal && mf_schedule(&m)==2 && mf_invariant(&m));
  m.executing=1; CHECK(mf_reap(&m,a)==CMX_INVALID); m.executing=2; CHECK(mf_reap(&m,a)==CMX_OK);
  a=add(&m,1,1); CHECK(a);
  cmx_mutex x,y; CHECK(cmx_mutex_init(&m.kernel,&x,0,0)==CMX_OK && cmx_mutex_init(&m.kernel,&y,0,0)==CMX_OK);
  CHECK(cmx_mutex_lock(&m.kernel,&x,1,cmx_forever())==CMX_OK && cmx_mutex_lock(&m.kernel,&y,1,cmx_forever())==CMX_OK);
  CHECK(mf_fault(&m,a,9,0x789)==CMX_NOT_OWNER && m.fatal && mf_schedule(&m)==-1);
  CHECK(x.owner==1 && y.owner==1 && m.fault_code[1]==9);
  /* Explicit supervisor reset requires exclusive stopped-world ownership.
   * Poison both resources; repair application state BEFORE releasing either.
   * This specimen permits no automatic resumption of the faulted owner. */
  x.closed=y.closed=1; x.owner=y.owner=CMX_NONE;
  m.kernel.task[1].owned_sync=0;
  CHECK(cmx_exit(&m.kernel,1,-1)==CMX_OK); m.fatal=0;
  CHECK(mf_schedule(&m)==2 && mf_invariant(&m));
  CHECK(cmx_mutex_lock(&m.kernel,&x,2,cmx_forever())==CMX_CLOSED);
  return 0;
}
static int timers(void) {
  struct mf_timers t,before; mf_handle_id a,b,c,out[MF_TIMERS]; size_t n;
  mf_timers_init(&t,UINT32_MAX-2);
  CHECK(mf_timer_arm(&t,2,&a)==CMX_OK && mf_timer_arm(&t,2,&b)==CMX_OK && mf_timer_arm(&t,0,&c)==CMX_OK);
  n=MF_TIMERS; CHECK(mf_timer_poll(&t,1,out,&n)==CMX_OK && n==1 && out[0]==c);
  n=MF_TIMERS; CHECK(mf_timer_poll(&t,2,out,&n)==CMX_OK && n==2 && out[0]==a && out[1]==b);
  CHECK(mf_timer_arm(&t,3,&a)==CMX_OK && mf_timer_cancel(&t,a)==CMX_OK);
  CHECK(mf_timer_arm(&t,4,&b)==CMX_OK && a!=b && mf_timer_cancel(&t,a)==CMX_INVALID);
  memcpy(&before,&t,sizeof t); n=MF_TIMERS;
  CHECK(mf_timer_poll(&t,UINT32_C(0x80000002),out,&n)==CMX_INVALID && !memcmp(&t,&before,sizeof t));
  CHECK(mf_timer_arm(&t,UINT32_C(0x80000002),&a)==CMX_INVALID);
  for(unsigned i=0;i<MF_TIMERS-1;++i) CHECK(mf_timer_arm(&t,4,&a)==CMX_OK);
  memcpy(&before,&t,sizeof t); CHECK(mf_timer_arm(&t,4,&a)==CMX_FULL && !memcmp(&t,&before,sizeof t));
  n=MF_TIMERS; CHECK(mf_timer_poll(&t,4,out,&n)==CMX_OK && n==MF_TIMERS && out[0]==b);
  t.next_sequence=UINT64_MAX; memcpy(&before,&t,sizeof t);
  CHECK(mf_timer_arm(&t,5,&a)==CMX_FULL && !memcmp(&t,&before,sizeof t));
  mf_timers_init(&t,0); t.timer[0].generation=UINT32_MAX;
  CHECK(mf_timer_arm(&t,0,&a)==CMX_OK); n=MF_TIMERS;
  CHECK(mf_timer_poll(&t,0,out,&n)==CMX_OK && n==1 && t.timer[0].retired);
  CHECK(mf_timer_arm(&t,1,&b)==CMX_OK && (b&31u)==2);
  CHECK(mf_due32(0,UINT32_MAX) && !mf_due32(UINT32_MAX,0));
  CHECK(mf_due64(0,UINT64_MAX) && !mf_due64(UINT64_MAX,0));
  CHECK(!mf_due64(0,UINT64_C(1)<<63));
  return 0;
}
static int timing(void) {
  uint32_t next,skip,counts; uint64_t rest;
  CHECK(mf_periodic(0,10,0,&next,&skip)==CMX_OK && next==10 && skip==0);
  CHECK(mf_periodic(10,10,13,&next,&skip)==CMX_OK && next==20 && skip==0);
  CHECK(mf_periodic(20,10,26,&next,&skip)==CMX_OK && next==30 && skip==0);
  CHECK(mf_periodic(30,10,57,&next,&skip)==CMX_OK && next==60 && skip==2);
  CHECK(mf_periodic(UINT32_MAX-4,10,2,&next,&skip)==CMX_OK && next==5);
  CHECK(mf_periodic(0,0,0,&next,&skip)==CMX_INVALID);
  CHECK(mf_counts(101,32768,0xffffff,&counts,&rest)==CMX_OK && counts==4 && rest==0);
  CHECK(mf_counts(101,32768,3,&counts,&rest)==CMX_OK && counts==3 && rest==10);
  CHECK(mf_counts(rest,32768,3,&counts,&rest)==CMX_OK && counts==1 && rest==0);
  CHECK(mf_counts(UINT64_MAX,2,999,&counts,&rest)==CMX_INVALID);
  CHECK(mf_counts(0,32768,999,&counts,&rest)==CMX_OK && counts==0 && rest==0);
  /* Independent integer inequalities avoid trusting a repeated ceil formula. */
  for(uint32_t hz=30001;hz<40000;hz+=997)
    for(uint32_t us=1;us<1200;us+=17) {
      CHECK(mf_counts(us,hz,100000,&counts,&rest)==CMX_OK);
      CHECK((uint64_t)counts*1000000>=(uint64_t)us*hz);
      CHECK((uint64_t)(counts-1)*1000000<(uint64_t)us*hz && rest==0);
    }
  return 0;
}
static int masks_sleep(void) {
  unsigned result=999;
  CHECK(mf_basepri(0,80,4,&result)==CMX_OK && result==80);
  CHECK(mf_basepri(result,48,4,&result)==CMX_OK && result==48);
  CHECK(mf_basepri(result,80,4,&result)==CMX_OK && result==48);
  CHECK(mf_basepri(0,48,3,&result)==CMX_INVALID && result==48);
  CHECK(mf_basepri(0,64,3,&result)==CMX_OK && result==64);
  CHECK(mf_basepri(0,1,8,&result)==CMX_OK && result==1);
  for(unsigned boundary=0;boundary<7;++boundary) {
    struct mf_sleep s; mf_sleep_init(&s);
    if(boundary==0) mf_sleep_wake(&s);
    CHECK(mf_sleep_arm(&s,UINT32_MAX-10,100)==CMX_OK);
    if(boundary==1 || boundary==2 || boundary==3 || boundary==4) mf_sleep_wake(&s);
    int slept=mf_sleep_enter(&s);
    if(boundary<=4) CHECK(!slept && s.ready); else CHECK(slept);
    if(boundary>=5) mf_sleep_wake(&s);
    CHECK(mf_sleep_finish(&s,29)==CMX_OK && s.elapsed==40 && s.ready);
    CHECK(mf_sleep_finish(&s,30)==CMX_INVALID && s.elapsed==40);
    CHECK(mf_sleep_arm(&s,100,100)==CMX_OK && !mf_sleep_enter(&s));
    CHECK(mf_sleep_finish(&s,200)==CMX_OK && s.elapsed==140);
  }
  struct mf_sleep s; mf_sleep_init(&s); CHECK(mf_sleep_arm(&s,0,10)==CMX_OK);
  CHECK(mf_sleep_finish(&s,11)==CMX_INVALID && s.elapsed==0);
  return 0;
}
static int helper(void) {
  uint64_t values[]={0,1,2,3,UINT32_MAX,UINT64_C(1)<<63,UINT64_MAX-1,UINT64_MAX};
  for(unsigned a=0;a<8;++a) for(unsigned b=0;b<8;++b)
    CHECK(mf_div64(values[a],values[b])==(values[b]?values[a]/values[b]:UINT64_MAX));
  uint64_t x=UINT64_C(0x7a12e45b93c6d809);
  for(unsigned n=0;n<20000;++n) {
    x^=x<<13; x^=x>>7; x^=x<<17; uint64_t a=x;
    x^=x<<13; x^=x>>7; x^=x<<17; uint64_t b=x|1u;
    CHECK(mf_div64(a,b)==a/b);
  }
  return 0;
}
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  struct {const char *name;int (*test)(void);} tests[]={
    {"admission",admission},{"lifecycle",lifecycle},{"scheduling",scheduling},
    {"faults",faults},{"timers",timers},{"timing",timing},{"masks-sleep",masks_sleep},{"helper",helper}};
  for(unsigned i=0;i<sizeof tests/sizeof tests[0];++i) if(!strcmp(argv[1],tests[i].name)) {
    int result=tests[i].test(); if(!result) printf("MF_PASS:%s\n",tests[i].name); return result;
  }
  return 2;
}
