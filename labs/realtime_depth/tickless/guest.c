#include "port.h"
static TQTimer timer;static TPClock clock;
static void say(const char *s){register unsigned r0 __asm__("r0")=4;register const void *r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");}
static void finish(unsigned code){unsigned args[2]={0x20026u,code};register unsigned r0 __asm__("r0")=0x20;register const void *r1 __asm__("r1")=args;__asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");for(;;){}}
static void number(uint32_t value){char b[11];unsigned i=10;b[i]=0;do{b[--i]=(char)('0'+value%10);value/=10;}while(value);say(b+i);say(" ");}
#define CHECK(x) do{if(!(x)){say("TP_CHECK:");number(__LINE__);say("\n");finish(1);}}while(0)
static unsigned events(void){unsigned value;__asm__ volatile("cpsid i":::"memory");value=timer.events;__asm__ volatile("cpsie i":::"memory");return value;}
static void wait_events(unsigned wanted){uint32_t start=tp_raw();unsigned prior=clock.interrupts;
 while((events()&wanted)!=wanted){
  CHECK((uint32_t)(tp_raw()-start)<2000000u);
  if(clock.interrupts!=prior){prior=clock.interrupts;enum tq_status s=tp_arm(&clock,0);CHECK(s==TQ_OK||s==TQ_RETRY);}
 }
}
static uint32_t collect(unsigned task){uint32_t when;enum cmx_result result;__asm__ volatile("cpsid i":::"memory");enum tq_status s=tq_collect(&timer,task,&when,&result);__asm__ volatile("cpsie i":::"memory");CHECK(s==TQ_OK&&result==CMX_OK);return when;}
void SysTick_Handler(void){if(tp_interrupt(&clock)!=TQ_OK){say("TP_IRQ_FAULT\n");finish(2);}}
void task_main(void){
 uint32_t origin=UINT32_MAX-250000u;CHECK(tp_init(&clock,&timer,origin)==TQ_OK);
 uint32_t a=origin+400000u,b=origin+600000u;
 CHECK(tp_at(&clock,0,a)==TQ_OK&&tp_at(&clock,1,a)==TQ_OK&&tp_at(&clock,2,b)==TQ_OK&&tp_arm(&clock,0)==TQ_OK);
 wait_events(3);uint32_t x=collect(0),y=collect(1);CHECK(x==y&&(uint32_t)(x-a)<50000u);
 CHECK(tp_arm(&clock,0)==TQ_OK);wait_events(4);uint32_t z=collect(2);CHECK((uint32_t)(z-b)<50000u);
 say("TP_TRACE wrap ");number(a);number(x);number(b);number(z);number(clock.interrupts);say("\n");tp_shutdown(&clock);

 CHECK(tp_init(&clock,&timer,0)==TQ_OK);a=tp_now(&clock)+500000u;CHECK(tp_at(&clock,0,a)==TQ_OK&&tp_arm(&clock,0)==TQ_OK);
 b=tp_now(&clock)+100000u;CHECK(tp_at(&clock,1,b)==TQ_OK&&tp_arm(&clock,0)==TQ_OK);
 wait_events(2);y=collect(1);CHECK((uint32_t)(y-b)<50000u&&!(events()&1));
 CHECK(tp_arm(&clock,0)==TQ_OK);wait_events(1);x=collect(0);CHECK((uint32_t)(x-a)<50000u);
 say("TP_TRACE earlier ");number(b);number(y);number(a);number(x);say("\n");tp_shutdown(&clock);

 CHECK(tp_init(&clock,&timer,0)==TQ_OK);a=tp_now(&clock)+100000u;CHECK(tp_at(&clock,0,a)==TQ_OK);
 CHECK(tp_arm(&clock,150000u)==TQ_RETRY);CHECK(events()&1);x=collect(0);CHECK((uint32_t)(x-a)<100000u);
 say("TP_TRACE pending ");number(a);number(x);number(clock.interrupts);say("\n");tp_shutdown(&clock);

 CHECK(tp_init(&clock,&timer,0)==TQ_OK);a=tp_now(&clock)+100000u;CHECK(tp_at(&clock,0,a)==TQ_OK&&tp_stop(&clock)==TQ_OK);
 uint32_t stopped_now=timer.kernel.now,stopped_sample=clock.resume.last;
 uint32_t before=tp_raw();while((uint32_t)(tp_raw()-before)<150000u){}
 CHECK(tp_wait(&clock,1,50000,0)==TQ_INVALID&&tp_at(&clock,1,a+50000u)==TQ_INVALID);
 CHECK(tp_interrupt(&clock)==TQ_INVALID&&tp_arm(&clock,0)==TQ_INVALID&&tp_stop(&clock)==TQ_INVALID);
 CHECK(timer.kernel.now==stopped_now&&clock.resume.last==stopped_sample&&timer.events==0&&clock.interrupts==0);
 CHECK(clock.interrupts==0&&tp_resume(&clock)==TQ_OK&&events()==1);x=collect(0);CHECK((uint32_t)(x-a)<100000u);
 CHECK(timer.kernel.now==clock.logical_origin+(clock.resume.last-clock.raw_origin));
 CHECK(tp_resume(&clock)==TQ_INVALID);
 uint32_t saved=timer.kernel.now;CHECK(tq_resume(&timer,&clock.resume,clock.resume.last)==TQ_OK&&timer.kernel.now==saved);
 say("TP_TRACE stopped ");number(a);number(x);number(clock.resume.last);number(clock.interrupts);say("\n");tp_shutdown(&clock);
 CHECK(tp_resume(&clock)==TQ_INVALID&&tp_wait(&clock,1,1,0)==TQ_INVALID&&tp_interrupt(&clock)==TQ_INVALID);
 say("TP_PASS:actual-systick-cmsdk-wrap-earlier-pending-stopped\n");finish(0);
}
void fault(void){say("TP_FAULT\n");finish(2);}
extern unsigned cl_stack_top,cl_data_load,cl_data_start,cl_data_end,cl_bss_start,cl_bss_end;
void cl_reset(void){unsigned *d=&cl_data_start,*s=&cl_data_load;while(d<&cl_data_end)*d++=*s++;for(d=&cl_bss_start;d<&cl_bss_end;)*d++=0;task_main();}
__attribute__((section(".vectors"),used)) void (*const vectors[])(void)={(void(*)(void))&cl_stack_top,cl_reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,SysTick_Handler};
