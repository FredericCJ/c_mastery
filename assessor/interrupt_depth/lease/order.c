#include "order.h"
void xd_compiler(void) { __asm__ volatile("" ::: "memory"); }
void xd_dmb(void) { __asm__ volatile("dmb sy" ::: "memory"); }
void xd_dsb(void) { __asm__ volatile("dsb sy" ::: "memory"); }
void xd_double_dsb(void) { __asm__ volatile("dsb sy\n\tdsb sy" ::: "memory"); }
