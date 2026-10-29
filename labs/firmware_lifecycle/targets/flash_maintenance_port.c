#include "flash_maintenance.h"
static uint32_t rd(void*c,uint32_t a){(void)c;return *(volatile uint32_t*)(uintptr_t)a;}
static void wr(void*c,uint32_t a,uint32_t v){(void)c;*(volatile uint32_t*)(uintptr_t)a=v;}
static void sync_io(void*c){(void)c;__asm__ volatile("dsb sy":::"memory");}
static void f_fresh(void*c,uint32_t a,uint32_t n){(void)c;(void)a;(void)n;uint32_t acr=rd(0,0x40023C00u);wr(0,0x40023C00u,acr&~0x600u);wr(0,0x40023C00u,(acr&~0x600u)|0x1800u);wr(0,0x40023C00u,acr&~0x600u);wr(0,0x40023C00u,acr);__asm__ volatile("dsb sy\n\tisb sy":::"memory");}
static void h_fresh(void*c,uint32_t a,uint32_t n){(void)c;__asm__ volatile("dsb sy":::"memory");for(uint32_t off=0;off<n;off+=32)wr(0,0xE000EF5Cu,a+off);__asm__ volatile("dsb sy\n\tisb sy":::"memory");}
struct fm_io fm_f439_io(void){return (struct fm_io){{0,rd,wr,sync_io},f_fresh};}
struct fm_io fm_h745_io(void){return (struct fm_io){{0,rd,wr,sync_io},h_fresh};}
