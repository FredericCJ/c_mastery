#include "target.h"
#include "boot_pair.h"
#ifndef DU_TARGET_MUTANT
#define DU_TARGET_MUTANT 0
#endif
extern const unsigned char app_start[],app_end[];
static uint32_t word(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int du_target(void){
 uint32_t base=(uint32_t)(uintptr_t)app_start;size_t n=(size_t)((uintptr_t)app_end-(uintptr_t)app_start);
 if(n>2048||n<10)return DU_INVALID;
 DuImage image={3,(uint32_t)n,base,word(app_start+4),word(app_start),25000000,3};DuMachine machine={16000000,1,0,0,0};DuPlan plan={0};
 if(du_prepare(&image,&machine,&plan)!=DU_INVALID)return DU_INVALID;
 machine.clock=25000000;machine.dma_owned=1;if(du_prepare(&image,&machine,&plan)!=DU_INVALID)return DU_INVALID;
 machine.dma_owned=0;if(du_prepare(&image,&machine,&plan)!=DU_OK)return DU_INVALID;
 /* Selected AN386 model clock, privileged MSP reset path, no IRQ/DMA/peripheral
    work has been started. Actual architectural transfer uses frozen provider. */
 if(DU_TARGET_MUTANT==1)plan.vector=0;
 fb_handoff(plan.vector,plan.stack,plan.entry);
}
