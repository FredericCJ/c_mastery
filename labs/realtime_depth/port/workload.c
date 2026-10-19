#include "port.h"
#include "cmx_port.h"
#ifndef SR_SCENARIO
#define SR_SCENARIO 0
#endif
static SRPort port;
static uint32_t stacks[3][512] __attribute__((aligned(8)));
static unsigned sampled[3], work[3], activations;
static struct {unsigned tick,ran,remaining,sleeping,wake,charged,closed;} trace[128];
static unsigned count;
static void say(const char *s) {
  register unsigned r0 __asm__("r0")=4;
  register const void *r1 __asm__("r1")=s;
  __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
static void number(unsigned n) {
  char digits[12];unsigned len=0;
  do {digits[len++]=(char)('0'+n%10);n/=10;} while(n);
  while(len){char one[2]={digits[--len],0};say(one);}
}
static void fail(unsigned line) {say("SR_TASK_CHECK:");number(line);say("\n");cmx_port_finish(1);}
#define CHECK(x) do {if(!(x))fail(__LINE__);} while(0)
static void sample(unsigned id) {
  uintptr_t psp;__asm__ volatile("mrs %0,psp":"=r"(psp));
  CHECK(cmx_machine.current==(int)id);
  /* A leaf task may have no software frame: the live descending-stack PSP
   * can equal its one-past end after exception return. It is not dereferenced. */
  CHECK(psp>(uintptr_t)&stacks[id][0]&&psp<=(uintptr_t)&stacks[id][512]);sampled[id]++;
}
void cmx_port_tick_hook(void) {
  CHECK(count<128&&cmx_port_ticks<120);
  uint32_t mask=cmx_port_lock();unsigned ran=(unsigned)cmx_machine.current;
  CHECK(ran<3);if(cmx_machine.task[ran].state!=CMX_RUNNING)ran=3;
  CHECK(sr_tick(&port,cmx_port_ticks+1)==CMX_OK);
  trace[count].tick=cmx_port_ticks+1;trace[count].ran=ran;
  trace[count].remaining=port.budget.core.remaining;trace[count].sleeping=port.sleeping;
  trace[count].wake=port.wake;trace[count].charged=port.charged;trace[count].closed=port.closed;count++;
  cmx_port_unlock(mask);
}
static void sleep_until(unsigned deadline) {
  uint32_t mask=cmx_port_lock();CHECK(cmx_sleep(&cmx_machine,0,cmx_at(deadline))==CMX_BLOCKED);
  cmx_port_yield();cmx_port_unlock(mask);
  CHECK(cmx_machine.task[0].result==CMX_OK);
}
static void server(void) {
  sample(1);
  if(SR_SCENARIO==3) {
    /* Intentionally violate the short-mask premise on this owned image.
     * A pending SysTick bit does not count elapsed timer wraps. */
    uint32_t mask=cmx_port_lock(),begin=cmx_port_ticks,before=port.charged;
    uint32_t previous=*(volatile uint32_t *)(uintptr_t)0xe000e018;
    unsigned wraps=0;
    while(wraps<5) {
      uint32_t now=*(volatile uint32_t *)(uintptr_t)0xe000e018;
      if(now>previous)wraps++;
      previous=now;work[1]++;
    }
    cmx_port_unlock(mask);
    mask=cmx_port_lock();unsigned ticks=cmx_port_ticks-begin,charges=port.charged-before;
    CHECK(wraps==5&&ticks==1&&charges==1);
    say("SR_COLLAPSE:wraps=");number(wraps);say(";observed_ticks=");number(ticks);say(";charges=");number(charges);say("\n");
    say("SR_TASK_PASS:owned-long-mask-counterexample;premise-violation-observed\n");
    (void)mask;cmx_port_finish(0);
  }
  for(;;){uint32_t mask=cmx_port_lock();if(port.closed){cmx_port_unlock(mask);return;}
    work[1]++;cmx_port_unlock(mask);__asm__ volatile("nop");}
}
static void background(void) {sample(2);for(;;){work[2]++;__asm__ volatile("nop":::"memory");}}
static void supervisor(void) {
  sample(0);
  unsigned busy=SR_SCENARIO==1?3:1;
  unsigned stop=SR_SCENARIO==2?20:70;
  for(unsigned release=0;release<stop;release+=5) {
    CHECK(cmx_port_ticks==release);activations++;
    while(cmx_port_ticks<release+busy){work[0]++;__asm__ volatile("nop":::"memory");}
    sleep_until(release+5);
  }
  uint32_t mask=cmx_port_lock();CHECK(sr_detach(&port)==CMX_INVALID);
  unsigned closing_sleep=cmx_machine.task[1].state==CMX_WAITING;
  if(SR_SCENARIO==2)CHECK(closing_sleep);
  CHECK(sr_close(&port)==CMX_OK);CHECK(sr_close(&port)==CMX_OK);
  CHECK(cmx_machine.task[1].state==CMX_READY);
  cmx_port_unlock(mask);sleep_until(cmx_port_ticks+2);
  mask=cmx_port_lock();CHECK(cmx_machine.task[1].state==CMX_EXITED);
  SRPort old=port;CHECK(sr_detach(&port)==CMX_OK);CHECK(cmx_reap(&cmx_machine,1)==CMX_OK);
  CHECK(sr_close(&port)==CMX_INVALID);CHECK(port.throttles>1&&port.charged>3);
  /* A stopped control-slot reuse is not a new port task: no context is
   * installed or dispatched. It tests the stale binding after real reap. */
  CHECK(cmx_task_add(&cmx_machine,1,0,(uintptr_t)stacks[1],sizeof stacks[1])==1);
  CHECK(sr_close(&old)==CMX_INVALID);
  CHECK(cmx_port_switches>10&&activations==stop/5);
  for(unsigned i=0;i<3;i++)CHECK(sampled[i]&&work[i]);
  unsigned charged=0;
  for(unsigned i=0;i<count;i++) {
    if(!trace[i].closed&&trace[i].ran==1)charged++;
    CHECK(trace[i].charged==charged);
    unsigned window=0;
    for(unsigned j=0;j<=i;j++)if(!trace[j].closed&&trace[j].ran==1&&trace[i].tick-trace[j].tick<10)window++;
    CHECK(window<=3);
  }
  /* Stop the timer before emitting observations; logging is not payload. */
  *(volatile uint32_t *)(uintptr_t)0xe000e010=0;
  for(unsigned i=0;i<count;i++) {
    say("SR_TRACE:");number(trace[i].tick);say(",");number(trace[i].ran);say(",");
    number(trace[i].remaining);say(",");number(trace[i].sleeping);say(",");number(trace[i].wake);say(",");
    number(trace[i].charged);say(",");number(trace[i].closed);say("\n");
  }
  say("SR_TASK_PASS:3-PSP;SVC-PendSV-SysTick;closing_sleep=");number(closing_sleep);
  say(";charged=");number(port.charged);say(";throttles=");number(port.throttles);say("\n");
  (void)mask;cmx_port_finish(0);
}
void cmx_workload_main(void) {
  cmx_port_init();cmx_machine.policy=CMX_FIXED;
  CHECK(cmx_port_task(supervisor,stacks[0],512,0)==0);
  CHECK(cmx_port_task(server,stacks[1],512,1)==1);
  CHECK(cmx_port_task(background,stacks[2],512,2)==2);
  CHECK(sr_init(&port,&cmx_machine,1,3,10)==CMX_OK);cmx_port_start();
}
