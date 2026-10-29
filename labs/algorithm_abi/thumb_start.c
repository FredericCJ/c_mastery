#include "bound.h"
extern uint32_t __stack_top,__data_start,__data_end,__data_load,__bss_start,__bss_end;
static void semihost(uint32_t op,const void *arg){register uint32_t r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
__attribute__((noreturn)) static void stop(unsigned code){uint32_t args[2]={0x20026u,code};semihost(4,code?"AB_THUMB_FAIL\n":"AB_THUMB_PASS\n");semihost(0x20,args);for(;;){}}
#define CHECK(x) do{if(!(x))stop(1);}while(0)
static void exercise(void){
 const uint32_t a[5]={0,7,7,0x80000000u,UINT32_MAX};uint32_t out,validation;
 CHECK(ab_checked(NULL,0,0,8,0,&out,&validation)==0 && !out);
 for(unsigned i=0;i<5;++i){uint32_t expected=0;while(expected<5 && a[expected]<a[i])++expected;struct ab_trace trace;CHECK(ab_trace(a,5,a[i],0,&trace)==0 && trace.index==expected && trace.count==3);CHECK(algo_abi_probe(a,5,a[i],&out)==1 && out==expected);CHECK(ab_callback_abi_probe(a,5,a[i],&out)==1 && out==expected);CHECK(ab_checked(a,5,5,a[i],0,&out,&validation)==0 && out==expected && validation==4);
  for(unsigned poison=0;poison<2;++poison){struct ab_context context={0,0,poison?0xdeadbeefu:0x13579bdu};CHECK(ab_callback_lower(a,5,a[i],ab_poison,&context)==expected && context.calls==3 && context.misaligned==0);}
 }
 uint32_t single[1]={0x80000000u};CHECK(algo_asm_lower(single,1,UINT32_MAX)==1 && algo_asm_lower(single,1,0)==0);
 struct ab_trace upper;CHECK(ab_trace(a,5,7,1,&upper)==0 && upper.index==3);
 const algo_pair records[3]={{0,1},{7,2},{7,3}};size_t index=99;CHECK(ab_records_checked(records,3,3,7,&index)==0 && index==1);CHECK(ab_records_checked(records,SIZE_MAX,SIZE_MAX,7,&index)==1 && index==1);
 uint32_t repeated[1024];for(unsigned i=0;i<1024;++i)repeated[i]=7;CHECK(algo_abi_probe(repeated,1024,7,&out)==1 && out==0);CHECK(algo_abi_probe(repeated,1024,8,&out)==1 && out==1024);
 uint32_t bad[2]={2,1};out=99;CHECK(ab_checked(bad,2,2,1,0,&out,&validation)==1 && out==99);
 stop(0);
}
void reset(void){uint32_t *src=&__data_load;for(uint32_t *p=&__data_start;p<&__data_end;) *p++=*src++;for(uint32_t *p=&__bss_start;p<&__bss_end;) *p++=0;exercise();}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const uintptr_t vectors[]={ (uintptr_t)&__stack_top,(uintptr_t)reset,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault,(uintptr_t)fault };
