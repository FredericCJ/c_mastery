/* Original reset/semihost pattern shared with the frozen algorithm_sort harness. */
#include "lookup.h"
extern uint32_t __stack_top,__data_start,__data_end,__data_load,__bss_start,__bss_end;
static void semihost(uint32_t op,const void*arg){register uint32_t r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
__attribute__((noreturn)) static void stop(unsigned code){uint32_t args[2]={0x20026u,code};semihost(4,code?"AL_THUMB_FAIL\n":"AL_THUMB_PASS\n");semihost(0x20,args);for(;;){}}
int al_failure(const char*s,unsigned line){semihost(4,s);(void)line;return 41;}
void al_emit_lookup(unsigned k,uint32_t key,const ALProbe*p){(void)k;(void)key;(void)p;}
void al_emit_match(unsigned t,size_t n,unsigned p,size_t m,const uint64_t*out,size_t used,uint64_t cost){(void)t;(void)n;(void)p;(void)m;(void)out;(void)used;(void)cost;}
void al_emit_stream(unsigned t,size_t n,unsigned p,size_t m,size_t split,const uint64_t*out,size_t used){(void)t;(void)n;(void)p;(void)m;(void)split;(void)out;(void)used;}
void reset(void){uint32_t*src=&__data_load;for(uint32_t*p=&__data_start;p<&__data_end;)*p++=*src++;for(uint32_t*p=&__bss_start;p<&__bss_end;)*p++=0;for(unsigned i=0;i<5;++i)if(al_test(i))stop(1);stop(0);}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const uintptr_t vectors[]={(uintptr_t)&__stack_top,(uintptr_t)reset,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault};
