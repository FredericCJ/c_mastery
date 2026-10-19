#include "model.h"
static void print(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
_Noreturn void cb_main(void){int result=cb_suite();print(result?"CB_GUEST_FAIL\n":"CB_GUEST_PASS:actual Cortex-M execution of original register model;not STM32 cache\n");unsigned exit[2]={0x20026u,(unsigned)result};register unsigned r0 __asm__("r0")=0x20;register unsigned*r1 __asm__("r1")=exit;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");for(;;)__asm__ volatile("wfi");}
