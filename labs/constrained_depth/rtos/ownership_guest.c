#include "target.h"
/* Deliberately unsolved learner integration; do not use private sources. */
void cr_run(void){cr_fail(__LINE__);}
void cr_irq0_body(unsigned a,unsigned b,unsigned c){(void)a;(void)b;(void)c;cr_fail(__LINE__);}
void cr_irq1_body(unsigned a,unsigned b,unsigned c){(void)a;(void)b;(void)c;cr_fail(__LINE__);}
