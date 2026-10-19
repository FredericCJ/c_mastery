#include <stdint.h>
#include "target.h"
extern uint32_t cl_stack_top,cl_data_start,cl_data_end,cl_data_load,cl_bss_start,cl_bss_end;
extern void df_memmanage(void);
static void output(const char*s){register uintptr_t op __asm__("r0")=4;register const char*arg __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(op):"r"(arg):"memory");}
static void done(unsigned n){uint32_t block[]={0x20026,n};register uintptr_t op __asm__("r0")=0x20;register void*arg __asm__("r1")=block;__asm__ volatile("bkpt 0xab":"+r"(op):"r"(arg):"memory");for(;;)__asm__ volatile("wfi");}
static void failed(void){output("DF_TARGET_CHECK:unexpected-vector\n");done(1);}
void cl_reset(void){uint32_t*s=&cl_data_load;for(uint32_t*d=&cl_data_start;d<&cl_data_end;++d)*d=*s++;for(uint32_t*d=&cl_bss_start;d<&cl_bss_end;++d)*d=0;int result=df_target();output(result?"DF_TARGET_CHECK:contract\n":"DF_TARGET_PASS:actual-MPU fixed-record owned-store captured-PC unchanged-guard intra-region-allowed\n");done(result?1:0);}
__attribute__((section(".vectors"),used)) const uintptr_t df_vectors[16]={[0]=(uintptr_t)&cl_stack_top,[1]=(uintptr_t)cl_reset,[2]=(uintptr_t)failed,[3]=(uintptr_t)failed,[4]=(uintptr_t)df_memmanage};
