#include "fault.h"
#include "target.h"
#ifndef DF_TARGET_MUTANT
#define DF_TARGET_MUTANT 0
#endif
_Static_assert(ATOMIC_INT_LOCK_FREE==2,"No handler atomic runtime helper");
#define REG(a) (*(volatile uint32_t*)(uintptr_t)(a))
extern uint32_t cl_stack_top;
extern unsigned char df_fault_instruction[],df_fault_resume[];
extern void df_store(uint32_t,volatile uint32_t*);
static DfCapture capture;
static volatile uint32_t sandbox[72] __attribute__((aligned(32)));
static volatile unsigned handler_count,handler_ok;
/* This is a synthetic acquisition token; the host envelope binds the actual ELF
 * SHA256 separately. Never label this fixed token an embedded ELF digest. */
static const unsigned char token[32]={1};
static void output(const char*s){register uintptr_t op __asm__("r0")=4;register const char*arg __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(op):"r"(arg):"memory");}
static void hex(uint32_t v){char text[10];for(unsigned i=0;i<8;++i)text[i]="0123456789abcdef"[(v>>(28-4*i))&15];text[8]=' ';text[9]=0;output(text);}
void df_handler(uint32_t*words,uint32_t er){
 uint32_t status=REG(0xe000ed28),sp=(uint32_t)(uintptr_t)words,top=(uint32_t)(uintptr_t)&cl_stack_top;
 unsigned readable=sp>=top-8192&&sp<=top-32&&(sp&3)==0;
 DfInput input={.cfsr=status,.hfsr=REG(0xe000ed2c),.bfar=REG(0xe000ed38),.mmfar=REG(0xe000ed34),.exc_return=er,.stack_address=sp,.words=words,.available=8,.readable=readable,.aligned=1,.stacking_fault=(status&0x3838u)!=0,.image=token};
 ++handler_count;int result=df_capture(&capture,&input,0,0);
 /* This continuation belongs only to the one original synthetic STR below. */
 if(result==DF_OK&&readable&&!(status&0x3838u)&&words[6]==(uint32_t)(uintptr_t)df_fault_instruction){handler_ok=1;words[6]=(uint32_t)(uintptr_t)df_fault_resume;}
 else for(;;)__asm__ volatile("wfi");
 REG(0xe000ed94)=0;__asm__ volatile("dsb\nisb":::"memory");REG(0xe000ed28)=0x82;
}
int df_target(void){
 unsigned regions=(REG(0xe000ed90)>>8)&255;if(!regions)return 1;
 REG(0xe000ed94)=0;for(unsigned i=0;i<regions;++i){REG(0xe000ed98)=i;REG(0xe000eda0)=0;}
 REG(0xe000ed98)=0;REG(0xe000ed9c)=0xffffffe0;uint32_t alignment=REG(0xe000ed9c)&0xffffffe0u;
 if((alignment&0x20)==0)return 2;
 for(unsigned i=0;i<72;++i)sandbox[i]=0xaaaaaaaau;
 df_init(&capture);handler_count=handler_ok=0;
 REG(0xe000ed28)=0x030fbfbbu;REG(0xe000ed2c)=0x40000002u;REG(0xe000ed14)|=1u<<9;REG(0xe000ed24)|=1u<<16;
 REG(0xe000ed9c)=(uint32_t)(uintptr_t)sandbox;REG(0xe000eda0)=(1u<<28)|(1u<<19)|(4u<<1)|1u;
 if(DF_TARGET_MUTANT!=1)REG(0xe000ed94)=5;
 __asm__ volatile("dsb\nisb":::"memory");
 sandbox[8]=0x55667788u;if(sandbox[8]!=0x55667788u||handler_count)return 3;
 df_store(0x12345678u,sandbox);
 REG(0xe000ed94)=0;__asm__ volatile("dsb\nisb":::"memory");
 if(handler_count!=1||!handler_ok||sandbox[0]!=0xaaaaaaaau)return 4;
 DfRecord record;unsigned secondary;if(df_read(&capture,&record,&secondary)!=DF_OK||secondary||record.cfsr!=0x82||record.mmfar!=(uint32_t)(uintptr_t)sandbox||!(record.valid&DF_FRAME)||record.frame[6]!=(uint32_t)(uintptr_t)df_fault_instruction)return 5;
 unsigned char code[2]={df_fault_instruction[0],df_fault_instruction[1]};DfAccess access;
 if(DF_TARGET_MUTANT==2)record.frame[1]+=4;
 if(df_access(&record,token,record.frame[6],code,2,&access)!=DF_OK||!access.precise||!access.write||access.effective_address!=record.mmfar||access.operand!=0x12345678u)return 6;
 output("DF_CAPTURE:");hex(record.version);hex(record.valid);hex(record.cfsr);hex(record.hfsr);hex(record.exc_return);hex(record.mmfar);for(unsigned i=0;i<8;++i)hex(record.frame[i]);output("\n");
 return 0;
}
