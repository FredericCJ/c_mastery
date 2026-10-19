/* Original model-only MPS2 startup, distinct from the F439 link audit. */
#include "bus.h"
extern uint32_t __stack_top,__data_start,__data_end,__data_load,__bss_start,__bss_end;
static void semihost(uint32_t op,const void*arg){register uint32_t r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
__attribute__((noreturn)) static void stop(unsigned code){uint32_t args[2]={0x20026u,code};semihost(4,code?"MB_THUMB_FAIL\n":"MB_THUMB_PASS\n");semihost(0x20,args);for(;;){}}
void mb_failure(const char*s,unsigned line){semihost(4,s);(void)line;}
void mb_emit(const char*s,const uint32_t*v,size_t n){(void)s;(void)v;(void)n;}
void reset(void){uint32_t*src=&__data_load;for(uint32_t*p=&__data_start;p<&__data_end;)*p++=*src++;for(uint32_t*p=&__bss_start;p<&__bss_end;)*p++=0;const char*cases[]={"topology","placement","lease","arbitration","bridge"};
#ifdef MB_TEST_CASE
if(mb_test(cases[MB_TEST_CASE]))stop(1);
#else
for(unsigned i=0;i<5;++i)if(mb_test(cases[i]))stop(1);
#endif
stop(0);}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const uintptr_t vectors[]={(uintptr_t)&__stack_top,(uintptr_t)reset,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault};
