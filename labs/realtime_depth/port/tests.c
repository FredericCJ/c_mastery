#include "port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"SR_CHECK:%u:%s\n",__LINE__,#x);exit(1);}} while (0)
static cmx_kernel k;
static struct sr_port s;
static void setup(uint32_t q, uint32_t p) {
  cmx_init(&k,CMX_FIXED);
  CHECK(cmx_task_add(&k,1,0,0x1000,1024)==0);
  CHECK(cmx_task_add(&k,2,0,0x2000,1024)==1);
  CHECK(sr_init(&s,&k,0,q,p)==CMX_OK);CHECK(cmx_schedule(&k)==0);
}
static void tick(uint32_t n) {CHECK(sr_tick(&s,n)==CMX_OK);cmx_tick(&k,n);CHECK(cmx_schedule(&k)>=0);}
static void basic(void) {
  setup(2,5);tick(1);CHECK(s.charged==1&&s.budget.core.remaining==1);
  tick(2);CHECK(k.current==1&&s.sleeping&&s.wake==6&&s.throttles==1);
  for(unsigned i=3;i<=6;i++)tick(i);
  CHECK(k.current==0&&s.charged==2&&s.budget.core.remaining==1);
  tick(7);CHECK(s.charged==3&&s.budget.core.remaining==1);
  CHECK(sr_close(&s)==CMX_OK);CHECK(sr_detach(&s)==CMX_INVALID);
  CHECK(cmx_exit(&k,0,0)==CMX_OK);CHECK(sr_detach(&s)==CMX_INVALID);
  CHECK(cmx_schedule(&k)==1);CHECK(sr_detach(&s)==CMX_OK);
  CHECK(cmx_reap(&k,0)==CMX_OK);CHECK(sr_tick(&s,8)==CMX_INVALID);
  puts("SR_BASIC:charge-throttle-wake-close-exit-detach-reap");
}
static void guards(void) {
  setup(1,5);struct sr_port before=s;cmx_kernel old=k;
  CHECK(sr_tick(&s,2)==CMX_INVALID);CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  k.sequence=UINT32_MAX;old=k;
  CHECK(sr_tick(&s,1)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  k.sequence=0;tick(1);CHECK(s.sleeping&&k.current==1);
  /* This sleep belongs to the adapter. Corrupting its sequence must not
   * turn close into cancellation of some unrelated operation. */
  k.task[0].wait_sequence++;before=s;old=k;
  CHECK(sr_close(&s)==CMX_BROKEN);CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  k.task[0].wait_sequence--;CHECK(sr_close(&s)==CMX_OK);CHECK(k.task[0].state==CMX_READY);
  CHECK(sr_close(&s)==CMX_OK);CHECK(sr_tick(&s,2)==CMX_OK);
  setup(1,5);s.charged=UINT32_MAX;before=s;old=k;
  CHECK(sr_tick(&s,1)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  setup(1,5);k.now=UINT32_MAX-5;s.last=k.now;s.budget.core.last=k.now;before=s;old=k;
  CHECK(sr_tick(&s,k.now+1)==CMX_FULL);CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  puts("SR_GUARDS:clock-sequence-counter-overflow-exact-sleep-ownership");
}
static void generation(void) {
  setup(1,5);k.task[0].generation++;struct sr_port before=s;cmx_kernel old=k;
  CHECK(sr_tick(&s,1)==CMX_INVALID);CHECK(sr_close(&s)==CMX_INVALID);
  CHECK(!memcmp(&s,&before,sizeof s)&&!memcmp(&k,&old,sizeof k));
  puts("SR_GENERATION:reused-slot-refused");
}
static void stream(void) {
  unsigned q,p,hp;
  while(scanf("%u %u %u",&q,&p,&hp)==3) {
    setup(q,p);
    for(unsigned t=1;t<=80;t++) {
      /* Deliberately chosen external HP preemption; it never changes the
       * server's wait or generation. The independent oracle replays it. */
      if(k.current==0&&t%7<hp){k.task[0].state=CMX_READY;k.current=1;k.task[1].state=CMX_RUNNING;}
      unsigned ran=k.current==0;tick(t);
      printf("%u %u %u %u %u %u %u\n",t,ran,s.budget.core.remaining,s.sleeping,s.wake,s.charged,s.throttles);
    }
  }
}
int main(int argc,char **argv) {
  CHECK(argc==2);
  if(!strcmp(argv[1],"basic"))basic();
  else if(!strcmp(argv[1],"guards"))guards();
  else if(!strcmp(argv[1],"generation"))generation();
  else if(!strcmp(argv[1],"stream"))stream();else CHECK(0);
  return 0;
}
