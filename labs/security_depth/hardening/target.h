#ifndef SH_TARGET_H
#define SH_TARGET_H
#include <stdint.h>
extern volatile uint32_t sh_failure_record;
extern uintptr_t __stack_chk_guard;
void sh_record_failure(void);
_Noreturn void __stack_chk_fail(void);
unsigned sh_target_probe(unsigned x);
void sh_target_touch(volatile unsigned char *p);
#endif
