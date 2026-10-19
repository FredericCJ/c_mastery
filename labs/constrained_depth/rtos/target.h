#ifndef CMASTERY_CONSTRAINED_RTOS_TARGET_H
#define CMASTERY_CONSTRAINED_RTOS_TARGET_H
#include <stdint.h>
void cr_text(const char *);
void cr_number(unsigned);
_Noreturn void cr_fail(unsigned);
_Noreturn void cr_done(void);
void cr_run(void);
void cr_irq0_body(unsigned,unsigned,unsigned);
void cr_irq1_body(unsigned,unsigned,unsigned);
int cr_fp_probe(unsigned);
#define CR_CHECK(x) do{if(!(x))cr_fail(__LINE__);}while(0)
#ifndef CR_TARGET_MUTANT
#define CR_TARGET_MUTANT 0
#endif
#ifndef CR_FP
#define CR_FP 1
#endif
#ifndef CR_EXTRA_TASK
#define CR_EXTRA_TASK 0
#endif
#endif
