/* Mechanical selected MMIO port; no policy or alternate hardware inferred. */
#include "selected_startup.h"
static uint32_t rd(void*c,uint32_t a){(void)c;return *(volatile uint32_t*)(uintptr_t)a;}
static void wr(void*c,uint32_t a,uint32_t n){(void)c;*(volatile uint32_t*)(uintptr_t)a=n;}
static void sync(void*c){(void)c;__asm__ volatile("dsb sy\n\tisb sy":::"memory");}
FHIO st_target_io(void){return (FHIO){0,rd,wr,sync};}
