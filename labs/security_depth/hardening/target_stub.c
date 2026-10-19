#include "target.h"
uintptr_t __stack_chk_guard;
volatile uint32_t sh_failure_record;
void sh_record_failure(void) { }
_Noreturn void __stack_chk_fail(void) { for (;;) { } }
