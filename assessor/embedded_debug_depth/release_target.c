#include "target.h"
#ifndef DT_TARGET_MUTANT
#define DT_TARGET_MUTANT 0
#endif
static struct dt_trace trace;
static volatile unsigned injected,done;
static volatile uint32_t cookie=42,zero;
static volatile uint32_t values[300];
__attribute__((section(".noinit"))) static struct dt_retained retained;
static uint32_t enter(void*v){
 (void)v;uint32_t mask;__asm__ volatile("mrs %0,primask":"=r"(mask));
 if(DT_TARGET_MUTANT!=1)__asm__ volatile("cpsid i":::"memory");
 if(injected){injected=0;*(volatile uint32_t*)0xe000e200u=2;__asm__ volatile("dsb\nisb":::"memory");}
 return mask;
}
static void leave(void*v,uint32_t mask){(void)v;__asm__ volatile("msr primask,%0"::"r"(mask):"memory");}
static struct dt_critical port={0,enter,leave};
void dt_irq0(void){injected=1;if(dt_irq_emit(&trace,port,1,100,10)!=DT_OK)done=99;else done|=1;}
void dt_irq1(void){if(dt_irq_emit(&trace,port,2,101,20)!=DT_OK)done=99;else done|=2;}
__attribute__((noinline)) static uint32_t workload(unsigned mode){
 uint32_t sum=0;struct dt_record record;
 for(unsigned i=0;i<300;i++){sum+=values[i];if(mode){(void)dt_irq_emit(&trace,port,3,i,i);if(mode==2)(void)dt_irq_take(&trace,port,&record);}}
 return sum;
}
int dt_target(void){
 if(cookie!=42||zero)return 1;
 if(retained.valid){
  if(retained.version!=1||retained.image!=42||retained.code!=123||retained.check!=(42u^123u^0x72657431u))return 1;
  dt_output("DT_RESET:retained-v1-current-token data42 bss0\n");retained.valid=0;return 0;
 }
 /* Observed capability and movement, never assume emulator CYCCNT support. */
 volatile uint32_t*demcr=(volatile uint32_t*)0xe000edfcu,*ctrl=(volatile uint32_t*)0xe0001000u,*cycles=(volatile uint32_t*)0xe0001004u;
 uint32_t original=*ctrl,a=0,b=0;*demcr|=1u<<24;
 if(!(original&(1u<<25))){*cycles=0;*ctrl=original|1u;a=*cycles;for(volatile unsigned i=0;i<1000;i++){}b=*cycles;}
 dt_output("DT_DWT:");dt_hex(original);dt_output(":");dt_hex(a);dt_output(":");dt_hex(b);dt_output("\n");
 *(volatile uint8_t*)0xe000e400u=0x80;*(volatile uint8_t*)0xe000e401u=0x40;
 *(volatile uint32_t*)0xe000e100u=3;*(volatile uint32_t*)0xe000e200u=1;
 __asm__ volatile("cpsie i\ndsb\nisb":::"memory");
 for(unsigned i=0;done!=3&&i<100000;i++)__asm__ volatile("nop":::"memory");
 if(done!=3)return 1;
 struct dt_record record;if(dt_irq_take(&trace,port,&record)!=DT_OK||record.kind!=1)return 1;
 if(dt_irq_take(&trace,port,&record)!=DT_OK||record.kind!=2)return 1;
 __asm__ volatile("cpsid i":::"memory");
 if(dt_irq_emit(&trace,port,4,0,0)!=DT_OK)return 1;
 uint32_t mask;__asm__ volatile("mrs %0,primask":"=r"(mask));if(mask!=1)return 1;
 (void)dt_irq_take(&trace,port,&record);
 volatile uint32_t*timer=(volatile uint32_t*)0x40000000u;timer[0]=0;timer[2]=0x00ffffffu;timer[3]=1;timer[0]=1;
 for(unsigned i=0;i<300;i++)values[i]=i;
 for(unsigned sample=0;sample<7;sample++)for(unsigned mode=0;mode<3;mode++){
  trace=(struct dt_trace){0};uint32_t before=timer[1],sum=workload(mode),after=timer[1];
  if(sum!=44850||before<after||before==after)return 1;
  dt_output("DT_COST:");dt_hex(sample);dt_output(":");dt_hex(mode);dt_output(":");dt_hex(before-after);dt_output(":");dt_hex(trace.dropped);dt_output("\n");
 }
 timer[0]=0;*(volatile uint32_t*)0xe000e180u=3;
 if(dt_record(&retained,42,123)!=DT_OK)return 1;
 if(DT_TARGET_MUTANT==2)retained.version=2;
 __asm__ volatile("dsb":::"memory");*(volatile uint32_t*)0xe000ed0cu=0x05fa0004u;__asm__ volatile("dsb":::"memory");
 for(;;)__asm__ volatile("wfi");
}
