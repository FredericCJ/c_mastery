#include "notifications_irq.h"
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define REG8(a) (*(volatile uint8_t *)(uintptr_t)(a))
extern uint32_t cl_stack_top,cl_data_start,cl_data_end,cl_data_load,cl_bss_start,cl_bss_end;
static cmx_kernel kernel;
static struct cd_notify notify;
static struct cdn_handle recipient;
static volatile unsigned interrupts,pend_calls,urgent;
static volatile uint32_t before_irq,entry_ticks[4],post_ticks[4],deferred_ticks;
static void semihost(uint32_t op,const void *arg) {
  register uint32_t r0 __asm__("r0")=op;
  register const void *r1 __asm__("r1")=arg;
  __asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t args[2]={0x20026u,status};
  semihost(4,status==0?"CDN_M4_PASS\n":status==1?"CDN_M4_CHECK\n":"CDN_M4_FAULT\n");
  semihost(0x20,args);for(;;){}
}
#define CHECK(c) do{if(!(c))stop(1);}while(0)
static unsigned ipsr(void){unsigned n;__asm__ volatile("mrs %0,ipsr":"=r"(n));return n;}
static unsigned primask(void){unsigned n;__asm__ volatile("mrs %0,primask":"=r"(n));return n;}
static void basepri(unsigned n){__asm__ volatile("msr basepri,%0\ndsb\nisb"::"r"(n):"memory");}
static uint32_t ticks(void){return REG32(0xe000e018u)&UINT32_C(0xffffff);}
static uint32_t elapsed(uint32_t a,uint32_t b){return(a-b)&UINT32_C(0xffffff);}
static void hex(uint32_t v){
  char text[10];const char *digits="0123456789abcdef";
  for(unsigned i=0;i<8;i++)text[i]=digits[(v>>(28-4*i))&15];
  text[8]='\n';text[9]=0;semihost(4,text);
}
static void irq0(void){
  CHECK(ipsr()==16);
  uint32_t started=ticks(); struct cdn_post out={99,98,97};
  CHECK(cdn_thread_wait(recipient,1,cmx_forever())==CMX_INVALID);
  if(urgent){
    CHECK(cdn_irq_post(recipient,1,&out)==CMX_INVALID && out.woke==99);
  }else{
    unsigned i=interrupts;const uint32_t batches[4]={3,2,3,4};
    uint32_t begin=ticks();
    int result=cdn_irq_post(recipient,batches[i],&out);
    post_ticks[i]=elapsed(begin,ticks());entry_ticks[i]=elapsed(before_irq,started);
    CHECK(result==(i==3?CMX_FULL:CMX_OK));
    if(i<3)CHECK(out.object_probes==1 && out.woke==(i==0));
    CHECK(kernel.current==0); /* Posting never schedules from this handler. */
  }
  interrupts++;
}
static void pendsv(void){
  CHECK(ipsr()==14);uint32_t begin=ticks(),saved=cdn_irq_save();
  CHECK(cmx_schedule(&kernel)==1);
  pend_calls++;cdn_irq_restore(saved);deferred_ticks=elapsed(begin,ticks());
}
static void fire(unsigned expected){
  before_irq=ticks();REG32(0xe000e200u)=1;
  __asm__ volatile("dsb\nisb":::"memory");
  for(unsigned i=0;i<100000 && interrupts!=expected;i++)__asm__ volatile("nop":::"memory");
  CHECK(interrupts==expected);
}
static void exercise(void){
  cmx_init(&kernel,CMX_FIXED);
  CHECK(cmx_task_add(&kernel,9,0,0x20006000,256)==0);
  CHECK(cmx_task_add(&kernel,3,0,0x20006200,256)==1);
  cdn_init(&notify,&kernel);CHECK(cdn_attach(&notify,1,8,&recipient)==CMX_OK);
  CHECK(cdn_thread_wait(recipient,1,cmx_forever())==CMX_BLOCKED && cmx_schedule(&kernel)==0);
  REG8(0xe000ed22u)=0xff;REG8(0xe000e400u)=CDN_KERNEL_THRESHOLD;REG32(0xe000e100u)=1;
  REG32(0xe000e014u)=UINT32_C(0xffffff);REG32(0xe000e018u)=0;REG32(0xe000e010u)=5;
  for(unsigned i=0;i<100000&&!ticks();i++)__asm__ volatile("nop":::"memory");
  CHECK(ticks()!=0);
  uint32_t saved=cdn_irq_save(),nested=cdn_irq_save();CHECK(saved==0&&nested==1&&primask()==1);
  cdn_irq_restore(nested);CHECK(primask()==1);cdn_irq_restore(saved);CHECK(primask()==0);
  basepri(0xc0); /* Allows IRQ0 priority0x80, holds PendSV priority0xff. */
  for(unsigned i=1;i<=4;i++)fire(i);
  CHECK(pend_calls==0 && notify.accepted==8 && notify.claimed==1 && kernel.task[1].notifications==7 && cdn_invariant(&notify));
  struct cdn_fault fault;CHECK(cdn_fault(recipient,&fault)==CMX_OK&&fault.count==4&&notify.rejected==4);
  CHECK(cdn_ack_fault(recipient,fault.sequence)==CMX_OK);
  basepri(0);
  for(unsigned i=0;i<100000&&pend_calls!=1;i++)__asm__ volatile("nop":::"memory");
  CHECK(pend_calls==1&&kernel.current==1);
  uint32_t count=999;
  CHECK(cdn_result(recipient,&count)==CMX_OK&&count==1);
  CHECK(cdn_thread_wait(recipient,3,cmx_forever())==CMX_OK&&cdn_result(recipient,&count)==CMX_OK&&count==3);
  CHECK(cdn_thread_wait(recipient,UINT32_MAX,cmx_forever())==CMX_OK&&cdn_result(recipient,&count)==CMX_OK&&count==4);
  urgent=1;REG8(0xe000e400u)=0;fire(5);
  struct cdn_handle old=recipient;
  CHECK(cdn_close(recipient)==CMX_OK&&cdn_detach(recipient)==CMX_OK);
  CHECK(cmx_exit(&kernel,1,0)==CMX_OK&&cmx_schedule(&kernel)==0&&cmx_reap(&kernel,1)==CMX_OK);
  CHECK(cmx_task_add(&kernel,3,0,0x20006200,256)==1&&cdn_attach(&notify,1,8,&recipient)==CMX_OK);
  struct cdn_post post;CHECK(cdn_post(old,1,&post)==CDN_STALE);
  for(unsigned i=0;i<4;i++){
    CHECK(entry_ticks[i]<UINT32_C(0x800000)&&post_ticks[i]<UINT32_C(0x800000));
    semihost(4,"CDN_IRQ_ENTRY_TICKS:");hex(entry_ticks[i]);semihost(4,"CDN_POST_TICKS:");hex(post_ticks[i]);
  }
  CHECK(deferred_ticks<UINT32_C(0x800000));
  semihost(4,"CDN_DEFERRED_TICKS:");hex(deferred_ticks);
  semihost(4,"CDN_IRQ_LEDGER:four-irqs;accepted8;refused4;one-pendsv;consumed8\n");
}
void cl_reset(void){
  uintptr_t source=(uintptr_t)&cl_data_load;
  for(uintptr_t p=(uintptr_t)&cl_data_start;p<(uintptr_t)&cl_data_end;p+=4,source+=4)*(uint32_t *)p=*(const uint32_t *)source;
  for(uintptr_t p=(uintptr_t)&cl_bss_start;p<(uintptr_t)&cl_bss_end;p+=4)*(uint32_t *)p=0;
  exercise();stop(0);
}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const void *cdn_vectors[]={
  &cl_stack_top,cl_reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,pendsv,fault,irq0};
