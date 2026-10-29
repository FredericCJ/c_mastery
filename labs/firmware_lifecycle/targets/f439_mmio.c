#include "f439_port.h"
static uint32_t rd(void*c,uint32_t a){(void)c;return *(volatile uint32_t*)(uintptr_t)a;}
static void wr(void*c,uint32_t a,uint32_t n){(void)c;*(volatile uint32_t*)(uintptr_t)a=n;}
static void sync(void*c){(void)c;__asm__ volatile("dsb sy\n\tisb sy":::"memory");}
static void h_fresh(void*c,uint32_t a,uint32_t n){(void)a;(void)n;f4_fresh((struct fh_io){c,rd,wr,sync});}
struct fm_io f4_target_io(void){return(struct fm_io){{0,rd,wr,sync},h_fresh};}
