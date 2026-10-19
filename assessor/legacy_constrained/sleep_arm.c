#include "sleep.h"
#ifndef CS_SLEEP_MUTANT
#define CS_SLEEP_MUTANT 0
#endif
unsigned cs_idle(unsigned event,volatile uint32_t*queued,void(*hook)(unsigned)){
 uint32_t ipsr,base,fault,saved;
 __asm__ volatile("mrs %0,ipsr":"=r"(ipsr));__asm__ volatile("mrs %0,basepri":"=r"(base));__asm__ volatile("mrs %0,faultmask":"=r"(fault));
 volatile uint32_t*scr=(volatile uint32_t*)UINT32_C(0xe000ed10);uint32_t control=*scr;
 if(!queued||event>1||ipsr||base||fault||(control&6))return 1;
 __asm__ volatile("mrs %0,primask\ncpsid i":"=r"(saved)::"memory");
 if(event){*scr=control|16;__asm__ volatile("dsb\nsev\nwfe":::"memory");}
 if(hook)hook(0);
 if(CS_SLEEP_MUTANT==1)*(volatile uint32_t*)UINT32_C(0xe000e280)=UINT32_C(0x300);
 if(!*queued&&!(*(volatile uint32_t*)UINT32_C(0xe000e200)&UINT32_C(0x300))){
  if(hook)hook(1);
  __asm__ volatile("dsb":::"memory");
  if(event)__asm__ volatile("wfe\nnop\nnop\nnop":::"memory");
  else __asm__ volatile("wfi\nnop\nnop\nnop":::"memory");
 }
 *scr=control;
 if(CS_SLEEP_MUTANT==2)saved=0;
 __asm__ volatile("msr primask,%0"::"r"(saved):"memory");return 0;
}
