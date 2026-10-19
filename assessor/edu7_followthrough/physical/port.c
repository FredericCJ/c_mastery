#include "physical.h"
#ifndef EP_PORT_MODEL
_Static_assert(sizeof(uintptr_t)==4,"selected F439 port address width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
static void barrier(void){__asm__ volatile("dsb" ::: "memory");}
#else
volatile uint32_t *ep_register(uint32_t);
void ep_barrier(void);
#define REG(a) (*ep_register(a))
static void barrier(void){ep_barrier();}
#endif
#ifndef EP_PORT_MUTANT
#define EP_PORT_MUTANT 0
#endif
static uint32_t offset(unsigned k){switch(k){case PU_SR:return 0;case PU_DR:return 4;case PU_BRR:return 8;case PU_CR1:return 12;case PU_CR2:return 16;default:return 20;}}
static uint32_t rd2(void *p,unsigned k){(void)p;
 if(k==PU_SOURCE){unsigned sws=(REG(0x40023808u)>>2)&3u;return sws<3&&((REG(0x40023800u)>>(sws==0?1:sws==1?17:25))&1u);}
 if(k==PU_GATE)return (REG(0x40023840u)>>17)&1u;
 if(k>=PU_SR&&k<=PU_CR3)return REG(0x40004400u+offset(k));
 return 0;
}
static void wr2(void *p,unsigned k,uintptr_t v){(void)p;
 if(k==PU_GATE)REG(0x40023840u)|=1u<<17;
 else if(k==PU_RESET){if(v)REG(0x40023820u)|=1u<<17;else REG(0x40023820u)&=~(1u<<17);}
 else if(k>=PU_SR&&k<=PU_CR3)REG(0x40004400u+offset(k))=(uint32_t)v;
}
static void drive2(void *p,unsigned on){(void)p;REG(0x40020018u)=1u<<(on?1:17);barrier();}
static R5Io io2(void){R5Io io={{0,rd2,wr2},0,drive2};return io;}
static void pins2(void){
 REG(0x40023830u)|=1u;(void)REG(0x40023830u);
 REG(0x40020018u)=1u<<17; /* inactive PA1 latch before output mode */
 REG(0x40020004u)&=~14u;REG(0x40020008u)&=~252u;
 REG(0x4002000cu)=(REG(0x4002000cu)&~252u)|(1u<<6);
 REG(0x40020020u)=(REG(0x40020020u)&~0xff00u)|(EP_PORT_MUTANT==1?0x6600u:0x7700u);
 REG(0x40020000u)=(REG(0x40020000u)&~252u)|164u;barrier();
}
int ep_f439_open(EpTx *s,const PuClock *clock,uint32_t baud,unsigned device,uint32_t now){
 return ep_f439_open_port(s,clock,baud,device,0,now);
}
int ep_f439_open_port(EpTx *s,const PuClock *clock,uint32_t baud,unsigned device,unsigned port,uint32_t now){
 if(!s||s->enabled||s->phase||device>R5_THVD1400||port>1)return EP_INVALID;
 R5Timing timing={device,0,1000,200000};
 if(port){pins2();return ep_open(s,io2(),clock,baud,&timing,now);}
 r5_f439_prepare();return ep_open(s,r5_f439_io(),clock,baud,&timing,now);
}
