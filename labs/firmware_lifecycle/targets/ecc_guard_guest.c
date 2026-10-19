#include "ecc_guard.h"
static void emit(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static _Noreturn void done(unsigned code){uint32_t arg[2]={0x20026,code};register unsigned r0 __asm__("r0")=0x20;register uint32_t*r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");for(;;)__asm__ volatile("wfi");}
_Noreturn void eg_unhandled(void){
#if EG_OUTSIDE
 emit("EG_GUEST_REFUSED:unrelated fault not swallowed\n");done(0);
#else
 emit("EG_GUEST_UNEXPECTED_FATAL\n");done(1);
#endif
}
void eg_guest_main(void){
 *(volatile uint32_t*)(uintptr_t)0xE000ED24u|=1u<<17; /* Dedicated startup, no IRQ mask. */
 uint32_t input=0x12345678u,out=0xA5A5A5A5u;
#if EG_OUTSIDE == 1
 out=*(volatile uint32_t*)(uintptr_t)0xFFFFFFF0u;emit("EG_GUEST_BAD_UNGUARDED_RETURN\n");done(out?1:1);
#elif EG_OUTSIDE == 2
 (void)eg_read_word((uint32_t)(uintptr_t)&input,(uint32_t*)(uintptr_t)0xFFFFFFF0u);emit("EG_GUEST_BAD_STORE_RETURN\n");done(1);
#endif
 if(eg_read_word((uint32_t)(uintptr_t)&input,&out)!=1||out!=input){emit("EG_GUEST_READ_FAIL\n");done(1);}
 out=0xA5A5A5A5u;if(eg_read_word(0xFFFFFFF0u,&out)!=0||out!=0xA5A5A5A5u||*(volatile uint32_t*)(uintptr_t)0xE000ED28u){emit("EG_GUEST_FAULT_FAIL\n");done(1);}
 if(eg_read_word((uint32_t)(uintptr_t)&input,&out)!=1||out!=input){emit("EG_GUEST_REUSE_FAIL\n");done(1);}emit("EG_GUEST_PASS:precise BusFault recovered,output preserved,next read succeeds\n");done(0);
}
