#include <stdint.h>
#ifndef DX_ABI_BAD
#define DX_ABI_BAD 0
#endif
extern uint32_t __stack_top;
extern int algo_abi_probe(const uint32_t *,uint32_t,uint32_t,uint32_t *);
static void semihost(uint32_t op,const void *arg){register uint32_t r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
void reset(void){static const uint32_t values[]={1,3,3,7};uint32_t out=99;int preserved=algo_abi_probe(values,4,3,&out);int bad=out!=1||preserved!=!DX_ABI_BAD;semihost(4,bad?"DX_THUMB:FAIL\n":DX_ABI_BAD?"DX_THUMB:clobber-detected-result1\n":"DX_THUMB:preserved-result1\n");uint32_t args[2]={0x20026u,(uint32_t)bad};semihost(0x20,args);for(;;){}}
static void fault(void){semihost(4,"DX_THUMB:FAULT\n");uint32_t args[2]={0x20026u,2};semihost(0x20,args);for(;;){}}
__attribute__((section(".vectors"),used)) const void *vectors[]={&__stack_top,reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};
