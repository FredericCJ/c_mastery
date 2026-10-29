#include "phy.h"
#ifndef EP_TARGET_MUTANT
#define EP_TARGET_MUTANT 0
#endif
#ifdef EP_TARGET_MODEL
int ep_test_read(uint32_t,uint32_t *);
int ep_test_write(uint32_t,uint32_t);
#endif
static int read_reg(void *p,unsigned reg,uint32_t *value){
 (void)p;if(reg>1 || !value)return 1;
 uint32_t address=UINT32_C(0x40028010)+reg*(EP_TARGET_MUTANT==1?8u:4u);
#ifdef EP_TARGET_MODEL
 return ep_test_read(address,value);
#else
 *value=*(volatile uint32_t *)(uintptr_t)address;return 0;
#endif
}
static int write_reg(void *p,unsigned reg,uint32_t value){
 (void)p;if(reg>1)return 1;
 uint32_t address=UINT32_C(0x40028010)+reg*(EP_TARGET_MUTANT==1?8u:4u);
#ifdef EP_TARGET_MODEL
 return ep_test_write(address,value);
#else
 *(volatile uint32_t *)(uintptr_t)address=value;return 0;
#endif
}
static uint32_t clock_reg(void *p){struct ep_mmio_clock *c=p;return c->clock(c->context);}
int ep_f439_bus(struct ep_mmio_clock *clock,struct ed_bus *out){
 if(!clock || !clock->clock || !out)return ED_INVALID;
 struct ed_bus result={clock,read_reg,write_reg,clock_reg};*out=result;return ED_OK;
}
