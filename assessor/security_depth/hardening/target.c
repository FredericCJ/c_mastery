#include "target.h"
/* Original fixed teaching guard, not an entropy or secrecy claim. */
uintptr_t __stack_chk_guard = UINT32_C(0x63c291a7);
volatile uint32_t sh_failure_record;
void sh_record_failure(void) {
#ifndef SH_MUTANT_RECORD
    sh_failure_record=UINT32_C(0x53484746);
#endif
}
_Noreturn void __stack_chk_fail(void) {
    sh_record_failure();
    for (;;) {
#if defined(__arm__) || defined(__thumb__)
        __asm__ volatile ("wfi");
#endif
    }
}
