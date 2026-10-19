#include <stdint.h>
#ifndef MW_BSS
#define MW_BSS 400
#endif
volatile unsigned char data[96]={1,2,3,4};
volatile unsigned char bss[MW_BSS];
volatile unsigned char retained[64] __attribute__((section(".retained")));
extern unsigned char _data_load[],_data_start[],_data_end[],_bss_start[],_bss_end[];
static void print(const char *s){register unsigned op __asm("r0")=4;register const char *p __asm("r1")=s;__asm volatile("bkpt 0xab"::"r"(op),"r"(p):"memory");}
static void stop(unsigned code){unsigned args[]={0x20026,code};register unsigned op __asm("r0")=0x20;register unsigned *p __asm("r1")=args;__asm volatile("bkpt 0xab"::"r"(op),"r"(p):"memory");for(;;){}}
static void startup(void){uintptr_t n=(uintptr_t)_data_end-(uintptr_t)_data_start;for(uintptr_t i=0;i<n;i++)_data_start[i]=_data_load[i];n=(uintptr_t)_bss_end-(uintptr_t)_bss_start;for(uintptr_t i=0;i<n;i++)_bss_start[i]=0;}
void Reset_Handler(void){for(unsigned i=0;i<64;i++)retained[i]=0xaa;startup();if(data[0]!=1||data[3]!=4)stop(1);for(unsigned i=0;i<MW_BSS;i++)if(bss[i])stop(2);for(unsigned i=0;i<64;i++)if(retained[i]!=0xaa)stop(3);data[0]=99;bss[0]=77;startup();if(data[0]!=1||bss[0]||retained[0]!=0xaa)stop(4);print("MW:QEMU:copy96:zero400:retained64:two-initializations:not-physical-reset\n");stop(0);}
extern unsigned char _stack_top[];
__attribute__((section(".vectors"),used)) const uintptr_t vectors[]={(uintptr_t)_stack_top,(uintptr_t)Reset_Handler};
