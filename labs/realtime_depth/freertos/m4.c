#include "checks.h"
static void say(const char *s){register unsigned r0 __asm__("r0")=4;register const void *r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");}
static void finish(unsigned code){unsigned args[2]={0x20026u,code};register unsigned r0 __asm__("r0")=0x20;register const void *r1 __asm__("r1")=args;__asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");for(;;){}}
void fc_fail(unsigned line){(void)line;say("FC_M4_CHECK\n");finish(1);}
void fc_done(const FCReport *r){if(fc_checks(r))fc_fail(0);say("FC_M4_PASS:queue-inheritance-timeout-cleanup\n");finish(0);}
void fc_fault(void){say("FC_M4_FAULT\n");finish(2);}
extern unsigned cl_stack_top,cl_data_load,cl_data_start,cl_data_end,cl_bss_start,cl_bss_end;
void cl_reset(void){unsigned *d=&cl_data_start,*s=&cl_data_load;while(d<&cl_data_end)*d++=*s++;for(d=&cl_bss_start;d<&cl_bss_end;)*d++=0;static FCReport report;
#ifdef FC_VENDOR
 *(volatile unsigned*)0xe000ed88u|=0xfu<<20;__asm__ volatile("dsb;isb":::"memory");
#endif
 fc_run(&report);fc_done(&report);}
#ifdef FC_VENDOR
extern void vPortSVCHandler(void),xPortPendSVHandler(void),xPortSysTickHandler(void),fc_irq(void);
#define FC_SVC vPortSVCHandler
#define FC_PEND xPortPendSVHandler
#define FC_TICK xPortSysTickHandler
#define FC_IRQ fc_irq
#else
#define FC_SVC fc_fault
#define FC_PEND fc_fault
#define FC_TICK fc_fault
#define FC_IRQ fc_fault
#endif
__attribute__((section(".vectors"),used)) void (*const vectors[])(void)={(void(*)(void))&cl_stack_top,cl_reset,fc_fault,fc_fault,fc_fault,fc_fault,fc_fault,0,0,0,0,FC_SVC,fc_fault,0,FC_PEND,FC_TICK,FC_IRQ};
