#include "transfer.h"
extern uint32_t __stack_top,__data_start,__data_end,__data_load,__bss_start,__bss_end;
static void semihost(uint32_t op,const void*arg){register uint32_t r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
__attribute__((noreturn)) static void stop(unsigned code){uint32_t args[2]={0x20026u,code};semihost(4,code?"CT_TARGET_FAIL\n":"CT_TARGET_PASS\n");semihost(0x20,args);for(;;){}}
int ct_failure(const char*s,unsigned n){semihost(4,s);(void)n;return 41;}
void ct_observation(const char*s,unsigned n,unsigned size){(void)s;(void)n;(void)size;}
void reset(void){uint32_t*src=&__data_load;for(uint32_t*p=&__data_start;p<&__data_end;)*p++=*src++;for(uint32_t*p=&__bss_start;p<&__bss_end;)*p++=0;stop(ct_tests(0)||ct_tests(1));}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const uintptr_t vectors[]={(uintptr_t)&__stack_top,(uintptr_t)reset,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault};
