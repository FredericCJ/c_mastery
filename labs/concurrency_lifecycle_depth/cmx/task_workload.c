#include "cl_cmx_task.h"
#ifndef CC_SCENARIO
#define CC_SCENARIO 0
#endif
static struct cc_queue queue;
static struct cc_handle handles[5], old;
static uint32_t stacks[6][768] __attribute__((aligned(8)));
static cmx_mutex mutex;
static cmx_cond condition;
static volatile unsigned producers_done, consumers_done, timeout_done, closed,
  mutex_held, close_observed, handler_checked;
static unsigned seen[3][20], received, sampled[6];
static void say(const char *s){
  register unsigned r0 __asm__("r0")=4;
  register const void *r1 __asm__("r1")=s;
  __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
static void fail(unsigned line){
  char digits[12];unsigned n=0;
  say("CC_TASK_CHECK:");do{digits[n++]=(char)('0'+line%10);line/=10;}while(line);
  while(n){char one[2]={digits[--n],0};say(one);}say("\n");cmx_port_finish(1);
}
#define CHECK(x) do{if(!(x))fail(__LINE__);}while(0)
static void sample(unsigned id){
  uintptr_t psp;__asm__ volatile("mrs %0,psp":"=r"(psp));
  CHECK(cmx_machine.current==(int)id);
  CHECK(psp>(uintptr_t)&stacks[id][0]&&psp<(uintptr_t)&stacks[id][768]);
  sampled[id]++;
}
static void release(unsigned id){uint32_t saved=cmx_port_lock();CHECK(cc_release(&handles[id])==CL_OK);cmx_port_unlock(saved);}
static void wait_closed(void){while(!closed)cmx_port_yield();}
static void restrictions(unsigned id){
  unsigned char bytes[5]={0};uint32_t saved=cmx_port_lock();
  int result=cc_task_send(handles[id],0,0,bytes,5,cmx_forever());
  cmx_port_unlock(saved);CHECK(result==CC_INVALID);
  uint32_t mask=128;
  __asm__ volatile("msr basepri,%0\ndsb\nisb"::"r"(mask):"memory");
  result=cc_task_send(handles[id],0,0,bytes,5,cmx_forever());mask=0;
  __asm__ volatile("msr basepri,%0\ndsb\nisb"::"r"(mask):"memory");
  CHECK(result==CC_INVALID);mask=1;
  __asm__ volatile("msr faultmask,%0\ndsb\nisb"::"r"(mask):"memory");
  result=cc_task_send(handles[id],0,0,bytes,5,cmx_forever());mask=0;
  __asm__ volatile("msr faultmask,%0\ndsb\nisb"::"r"(mask):"memory");
  CHECK(result==CC_INVALID);
}
void cmx_port_tick_hook(void){
  if(cmx_port_ticks>1000)fail(__LINE__);
  int current=cmx_machine.current;
  if(!handler_checked&&current>=0&&current<5&&handles[current].owner&&cmx_machine.task[current].state==CMX_RUNNING){
    unsigned char bytes[5]={0};
    CHECK(cc_task_send(handles[current],0,0,bytes,5,cmx_forever())==CC_INVALID);handler_checked=1;
  }
}
static void producer(unsigned id){
  sample(id);restrictions(id);
  while(!handler_checked)cmx_port_yield();
  /* After observing handler rejection, this case deliberately becomes
   * cooperative. A missing blocking PendSV cannot be hidden by a tick. */
  uint32_t timer_saved=cmx_port_lock();*(volatile uint32_t *)(uintptr_t)0xe000e010=0;
  *(volatile uint32_t *)(uintptr_t)0xe000ed04=1u<<25;
  cmx_port_unlock(timer_saved);
  for(unsigned sequence=0;sequence<20;sequence++){
    unsigned char bytes[4]={(unsigned char)id,(unsigned char)sequence,0x55,0xaa};
    CHECK(cc_task_send(handles[id],id,sequence,bytes,4,cmx_forever())==CL_OK);
    bytes[0]=0xff;bytes[1]=0xee;sample(id);cmx_port_yield();
  }
  uint32_t saved=cmx_port_lock();producers_done++;
  if(producers_done==3){CHECK(cc_close(&queue,0)==CL_OK);closed=1;}
  cmx_port_unlock(saved);wait_closed();release(id);
}
static void consumer(unsigned id){
  sample(id);for(;;){struct cl_message message={0};int result=cc_task_receive(handles[id],4,cmx_forever(),&message);
    if(result==CL_CLOSED)break;
    CHECK(result==CL_OK);CHECK(message.producer<3&&message.sequence<20&&message.length==4);
    CHECK(message.bytes[0]==message.producer&&message.bytes[1]==message.sequence&&message.bytes[2]==0x55&&message.bytes[3]==0xaa);
    uint32_t saved=cmx_port_lock();CHECK(!seen[message.producer][message.sequence]);seen[message.producer][message.sequence]=1;received++;cmx_port_unlock(saved);
    sample(id);cmx_port_yield();
  }
  uint32_t saved=cmx_port_lock();consumers_done++;cmx_port_unlock(saved);release(id);
}
static void timeout_task(unsigned id){
  sample(id);
  if(id<2){unsigned char byte=(unsigned char)(40+id);
    int result=cc_task_send(handles[id],id,0,&byte,1,id?cmx_at(cmx_port_ticks+8):cmx_forever());
    CHECK(result==(id?CC_TIMEOUT:CL_CLOSED));
    if(id){timeout_done=1;wait_closed();}
  }else if(id==2){
    while(!timeout_done)cmx_port_yield();
    uint32_t saved=cmx_port_lock();CHECK(queue.bytes.accepted==2&&queue.bytes.count==2);
    CHECK(cmx_machine.task[0].state==CMX_WAITING);CHECK(cc_close(&queue,1)==CL_OK);closed=1;cmx_port_unlock(saved);
  }else{wait_closed();struct cl_message m={.producer=777};CHECK(cc_task_receive(handles[id],4,cmx_forever(),&m)==CL_CLOSED);CHECK(m.producer==777);}
  sample(id);release(id);
}
static void condition_task(unsigned id){
  sample(id);
  if(id==0){
    CHECK(cc_task_condition(handles[0],&condition,&mutex,cmx_at(cmx_port_ticks+8))==CC_TIMEOUT);
    CHECK(mutex.owner==0&&close_observed);uint32_t saved=cmx_port_lock();
    CHECK(cc_release(&handles[0])==CL_BUSY);cmx_port_unlock(saved);
    CHECK(cc_task_condition_unlock(handles[0])==CL_OK);
  }else if(id==1){
    while(cmx_machine.task[0].wait!=CMX_W_COND)cmx_port_yield();
    uint32_t saved=cmx_port_lock();CHECK(cmx_mutex_lock(&cmx_machine,&mutex,1,cmx_forever())==CMX_OK);mutex_held=1;cmx_port_unlock(saved);
    while(!close_observed)cmx_port_yield();
    saved=cmx_port_lock();CHECK(cmx_mutex_unlock(&cmx_machine,&mutex,1)==CMX_OK);cmx_port_unlock(saved);
  }else if(id==2){
    while(!mutex_held||cmx_machine.task[0].wait!=CMX_W_MUTEX)cmx_port_yield();
    uint32_t saved=cmx_port_lock();CHECK(cmx_machine.task[0].pending==CMX_TIMEOUT);
    CHECK(cc_close(&queue,0)==CL_OK);closed=1;CHECK(cc_release(&handles[0])==CL_BUSY);CHECK(cc_destroy(&queue)==CL_BUSY);close_observed=1;cmx_port_unlock(saved);
  }else wait_closed();
  sample(id);release(id);
}
static void entry(unsigned id){
  if(CC_SCENARIO==0){if(id<3)producer(id);else consumer(id);}
  else if(CC_SCENARIO==1)timeout_task(id);else condition_task(id);
}
static void task0(void){entry(0);}static void task1(void){entry(1);}static void task2(void){entry(2);}
static void task3(void){entry(3);}static void task4(void){entry(4);}
static void coordinator(void){
  sample(5);for(;;){unsigned done=0;uint32_t saved=cmx_port_lock();
    for(unsigned i=0;i<5;i++)done+=cmx_machine.task[i].state==CMX_EXITED;
    cmx_port_unlock(saved);if(done==5)break;cmx_port_yield();}
  uint32_t saved=cmx_port_lock();
  CHECK(cmx_port_switches>12);for(unsigned i=0;i<6;i++)CHECK(sampled[i]);
  CHECK(queue.bytes.accepted==queue.bytes.delivered+queue.bytes.discarded+queue.bytes.count);
  if(CC_SCENARIO==0){CHECK(received==60&&producers_done==3&&consumers_done==2);CHECK(queue.bytes.accepted==60&&queue.bytes.delivered==60&&queue.bytes.discarded==0);for(unsigned p=0;p<3;p++)for(unsigned s=0;s<20;s++)CHECK(seen[p][s]);}
  if(CC_SCENARIO==1)CHECK(queue.bytes.accepted==2&&queue.bytes.discarded==2&&queue.bytes.delivered==0);
  if(CC_SCENARIO==2)CHECK(close_observed&&mutex.owner==CMX_NONE);
  for(unsigned i=0;i<5;i++)CHECK(cmx_reap(&cmx_machine,i)==CMX_OK);
  CHECK(cc_destroy(&queue)==CL_OK);CHECK(cc_restart(&queue)==CL_OK);struct cc_ticket ignored={0};CHECK(cc_send(old,0,0,0,0,cmx_forever(),&ignored)==CL_STALE);
  cmx_port_unlock(saved);
  /* Keep a live registered task until its first tick so handler rejection is
   * observed in each scenario; all scenarios naturally exceed eight ticks
   * except the throughput case, whose producer startup waits for it. */
  CHECK(handler_checked);say("CC_TASK_PASS:6-distinct-PSP-stacks;PendSV-resume;reap-after-exit;generation-retirement\n");cmx_port_finish(0);
}
void cmx_workload_main(void){
  cmx_port_init();void (*entries[6])(void)={task0,task1,task2,task3,task4,coordinator};
  for(unsigned i=0;i<6;i++)CHECK(cmx_port_task(entries[i],stacks[i],768,1)==(int)i);
  CHECK(cc_init(&queue,&cmx_machine)==CL_OK);for(unsigned i=0;i<5;i++)CHECK(cc_register(&queue,i,&handles[i])==CL_OK);old=handles[0];
  if(CC_SCENARIO==1){struct cc_ticket ticket;unsigned char byte=9;for(unsigned i=0;i<2;i++){CHECK(cc_send(handles[2],9,i,&byte,1,cmx_forever(),&ticket)==CL_OK);CHECK(cc_result(ticket,0)==CL_OK);}}
  if(CC_SCENARIO==2){CHECK(cmx_mutex_init(&cmx_machine,&mutex,0,0)==CMX_OK);CHECK(cmx_mutex_lock(&cmx_machine,&mutex,0,cmx_forever())==CMX_OK);}
  cmx_port_start();
}
