#include "ecc_guard.h"
static volatile uint32_t active,address;
extern const unsigned char eg_load_insn[],eg_abort[];
extern int eg_raw(uint32_t,uint32_t*);
__attribute__((weak,noreturn)) void eg_unhandled(void){for(;;)__asm__ volatile("wfi");}
int eg_read_word(uint32_t a,uint32_t*out){
 if(!out||(a&3u)||active||*(volatile uint32_t*)(uintptr_t)0xE000ED28u)return 0;
 address=a;active=1;__asm__ volatile("dmb sy":::"memory");int r=eg_raw(a,out);active=0;return r;
}
int eg_handle(uint32_t*frame,uint32_t ret){
 uint32_t status=*(volatile uint32_t*)(uintptr_t)0xE000ED28u,bfar=*(volatile uint32_t*)(uintptr_t)0xE000ED38u;
 if(!active||!eg_frame(frame,ret,status,bfar,(uint32_t)(uintptr_t)eg_load_insn,(uint32_t)(uintptr_t)eg_abort,address))return 0;
 *(volatile uint32_t*)(uintptr_t)0xE000ED28u=0x8200u;__asm__ volatile("dsb sy\n\tisb sy":::"memory");return 1;
}
