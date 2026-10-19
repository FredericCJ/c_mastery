#include <stdint.h>
_Noreturn void st_main(void){for(;;)__asm__ volatile("wfi");}
_Noreturn void th_report_main(uint32_t result,uint32_t generation){(void)result;(void)generation;for(;;)__asm__ volatile("wfi");}
