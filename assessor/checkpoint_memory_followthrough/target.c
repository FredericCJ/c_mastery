#include "checks.h"
static void emit(const char *s) {
    register unsigned r0 __asm__("r0")=4;register const char *r1 __asm__("r1")=s;
    __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
_Noreturn static void stop(unsigned rc) {
    unsigned args[2]={0x20026,rc};register unsigned r0 __asm__("r0")=0x20;register unsigned *r1 __asm__("r1")=args;
    __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");for(;;){}
}
void p1_fault(void) {emit("P1_TARGET_FAULT\n");stop(2);}
void p1_target_main(void) {
    if(p1_checks()) {emit("P1_TARGET_FAIL\n");stop(1);}
    emit("P1_TARGET_PASS traces=6561 memory_core explicit_runtime\n");stop(0);
}
