#include "power.h"
static const uintptr_t addresses[CS_REG_COUNT]={0x40023800,0x40023808,0x40023840,0x40007000,0x40003000,0x40003004,0x40003008,0x4000300c,0xe000ed10,0xe0042004,0xe000e010};
static uint32_t read_reg(void*c,enum cs_reg r){(void)c;return *(volatile uint32_t*)addresses[r];}
static void write_reg(void*c,enum cs_reg r,uint32_t v){(void)c;*(volatile uint32_t*)addresses[r]=v;(void)*(volatile uint32_t*)addresses[r];__asm__ volatile("dsb":::"memory");}
static void idle(void*c){(void)c;__asm__ volatile("dsb\nwfi\nnop\nnop\nnop":::"memory");}
struct cs_bus cs_f439_bus(void){return (struct cs_bus){0,read_reg,write_reg,idle};}
enum cs_status cs_f439_stop(struct cs_stop*s,unsigned premises){
 uint32_t ipsr,base,fault,saved;
 __asm__ volatile("mrs %0,ipsr":"=r"(ipsr));__asm__ volatile("mrs %0,basepri":"=r"(base));__asm__ volatile("mrs %0,faultmask":"=r"(fault));
 if(ipsr||base||fault)return CS_INVALID;
 __asm__ volatile("mrs %0,primask\ncpsid i":"=r"(saved)::"memory");
 struct cs_bus b=cs_f439_bus();enum cs_status result=cs_stop_enter(s,&b,premises);
 __asm__ volatile("msr primask,%0"::"r"(saved):"memory");return result;
}
