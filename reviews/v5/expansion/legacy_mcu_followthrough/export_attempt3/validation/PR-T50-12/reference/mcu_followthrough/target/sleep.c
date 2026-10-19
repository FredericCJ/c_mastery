#include "sleep.h"
#define R(a) (*(volatile uint32_t *)(uintptr_t)(a))
volatile uint32_t mf_sleep_irqs,mf_sleep_early;
static volatile unsigned ready;
static uint32_t lock(void){uint32_t p;__asm__ volatile("mrs %0,primask\ncpsid i":"=r"(p)::"memory");return p;}
static void unlock(uint32_t p){__asm__ volatile("msr primask,%0\ndsb\nisb"::"r"(p):"memory");}
void mf_clock_init(void){R(0x40000000)=0;R(0x40000008)=UINT32_MAX;R(0x40000000)=1;
 R(0xe000e100)=1u<<9;R(0xe000ed20)=(R(0xe000ed20)&0x00ffffffu)|0x80000000u;
 __asm__ volatile("cpsie i\ndsb\nisb":::"memory");}
uint32_t mf_clock_now(void){return UINT32_MAX-R(0x40000004);}
void Timer1_Handler(void){R(0x40001000)=0;R(0x4000100c)=1;ready=1;mf_sleep_irqs++;}
void SysTick_Handler(void){R(0xe000e010)=0;ready=1;mf_sleep_early++;}
static void inject(unsigned wanted,unsigned at){if(wanted==at){R(0xe000ed04)=1u<<26;__asm__ volatile("dsb\nisb":::"memory");}}
int mf_nap_finish(MfNap *s){if(!s->armed||s->credited)return 0;
 R(0x40001000)=0;uint32_t elapsed=mf_clock_now()-s->start;
 mf_require(elapsed<0x80000000u,201);s->elapsed=elapsed;s->remaining=elapsed>=s->requested?0:s->requested-elapsed;
 s->armed=0;s->credited=1;return 1;}
void mf_nap(MfNap *s,uint32_t count,unsigned boundary){
 mf_require(count>100&&count<0x80000000u&&boundary<=7,202);
 *s=(MfNap){0};ready=0;R(0xe000e010)=0;R(0x40001000)=0;R(0x4000100c)=1;
 R(0xe000e280)=1u<<9;R(0xe000ed04)=1u<<25;inject(boundary,0);
 uint32_t p=lock();inject(boundary,1);s->start=mf_clock_now();s->requested=count;s->armed=1;
 inject(boundary,2);R(0x40001008)=0;R(0x40001004)=count;R(0x40001000)=9;
 inject(boundary,3);
 unsigned pending=ready||(R(0xe000ed04)&(1u<<26))||(R(0xe000e200)&(1u<<9));
 inject(boundary,4);
 if(!pending){
  if(boundary==6){R(0xe000e014)=count/4u-1u;R(0xe000e018)=0;R(0xe000e010)=7;}
  inject(boundary,5);
#ifdef MF_STOP_CLOCK
  /* Deliberately violates the continuously running elapsed-source premise. */
  R(0x40000000)=0;
#endif
  s->slept=1;__asm__ volatile("dsb\nwfi\nisb":::"memory");
 }
 s->wake_irq=(R(0xe000e200)&(1u<<9))!=0;
 mf_require(mf_nap_finish(s),203);
 R(0x4000100c)=1;R(0xe000e280)=1u<<9;unlock(p);
}
