#include "port.h"
static uint32_t stacks[4][1024] __attribute__((aligned(8)));
static MfHandle handles[4];
static MfProbe probes[2];
static cmx_sem ready;
static cmx_mutex owned[2];
static volatile unsigned complete[2],resumed,hook_refusals,coalesced;
static volatile uint32_t guard[8] __attribute__((aligned(32)));
static uint32_t mask_empty,mask_body;
void mf_tick_hook(void){mf_require(mf_take(&ready,cmx_forever())==CMX_INVALID,120);hook_refusals++;}
static unsigned basepri(void){unsigned x;__asm__ volatile("mrs %0,basepri":"=r"(x));return x;}
static void probe(unsigned id){MfProbe *p=&probes[id];for(unsigned n=0;n<5;n++){
 mf_async_probe(p);for(unsigned j=0;j<8;j++)mf_require(p->integer[j]==p->observed_integer[j],130+j);
#if MF_FP
 for(unsigned j=0;j<16;j++)mf_require(p->high_fp[j]==p->observed_fp[j],140+j);
 mf_require(p->fpscr==p->observed_fpscr,156);
 mf_require(p->rounded==0x3f800000u+id,186);
#endif
 }complete[id]=1;}
static void first(void){
#ifdef MF_BAD_CANARY
 stacks[0][0]=0;
#endif
 unsigned p=mf_lock();mf_require(mf_take(&ready,cmx_forever())==CMX_INVALID,157);mf_unlock(p);
 unsigned outer=mf_mask_enter(80);mf_require(basepri()==80,158);
 unsigned inner=mf_mask_enter(48);mf_require(basepri()==48,159);mf_mask_leave(inner);mf_require(basepri()==80,160);
 inner=mf_mask_enter(112);mf_require(basepri()==80,161);mf_mask_leave(inner);mf_require(basepri()==80,162);
 mf_require(mf_take(&ready,cmx_forever())==CMX_INVALID,163);mf_mask_leave(outer);mf_require(basepri()==0,164);
 uint32_t before=MF_REG(0xe000e018),after=MF_REG(0xe000e018);mask_empty=before>=after?before-after:before+10000-after;
 before=MF_REG(0xe000e018);p=mf_mask_enter(80);for(volatile unsigned n=0;n<23;n++)__asm__ volatile("nop");
#ifdef MF_LOG_MASK
 mf_text("MF deliberately measured logging inside mask\n");mf_number(0x12345678);
#endif
 mf_mask_leave(p);after=MF_REG(0xe000e018);mask_body=before>=after?before-after:before+10000-after;
 mf_require(mf_take(&ready,cmx_at(mf_ticks+10))==CMX_OK,165);resumed=1;
 p=mf_lock();uint32_t switches=mf_switches,requests=mf_pend_requests;mf_pend();mf_pend();mf_pend();
 mf_require(mf_switches==switches&&mf_pend_requests==requests+3,166);coalesced=1;mf_unlock(p);
 probe(0);
}
static void second(void){mf_require(mf_give(&ready)==CMX_OK,167);probe(1);}
static void broken(void){
#ifdef MF_OWNER_FAULT
 unsigned p=mf_lock();unsigned t=(unsigned)mf_machine.executing;
 mf_require(cmx_mutex_lock(&mf_machine.kernel,&owned[0],t,cmx_forever())==CMX_OK,168);
 mf_require(cmx_mutex_lock(&mf_machine.kernel,&owned[1],t,cmx_forever())==CMX_OK,169);mf_unlock(p);
#endif
#ifdef MF_MPU_FAULT
 MF_REG(0xe000ed98)=0;MF_REG(0xe000ed9c)=(uint32_t)(uintptr_t)guard;
 MF_REG(0xe000eda0)=(1u<<28)|(4u<<1)|1u;MF_REG(0xe000ed94)=5;
 __asm__ volatile("dsb\nisb":::"memory");guard[0]=123;
#else
 guard[0]=1;__asm__ volatile("udf #71");
#endif
 mf_require(0,170);
}
static void supervisor(void){
 while(!complete[0]||!complete[1]||!mf_fault_count){mf_require(mf_ticks<200,171);mf_pend();}
 unsigned p=mf_lock();MF_REG(0xe000e010)=0;
 mf_require(resumed&&coalesced&&hook_refusals&&mf_handler_alignment,172);
 mf_require(mf_fault_count==1&&mf_fault_pc!=0,173);
#ifdef MF_MPU_FAULT
 mf_require((mf_fault_cfsr&0x82u)==0x82u&&guard[0]==0,174);
#else
 mf_require((mf_fault_cfsr&(1u<<16))!=0&&guard[0]==1,175);
#endif
 int faultslot=mf_lookup(&mf_machine,handles[2]);mf_require(faultslot==3&&mf_machine.kernel.task[3].state==CMX_EXITED,176);
 int result=42;mf_require(mf_join(&mf_machine,handles[3],handles[2],cmx_forever(),&result)==CMX_OK&&result==-1,177);
 mf_require(mf_reap(&mf_machine,handles[2])==CMX_OK&&mf_lookup(&mf_machine,handles[2])==-1,178);
 mf_require(mf_machine.executing==4&&mf_machine.kernel.current==4,179);
#if MF_FP
 mf_require(mf_fp_frames>0&&mf_lazy_after==0,180);
#ifdef MF_EAGER
 mf_require(mf_lazy_before==0,181);
 mf_require(mf_fp_before_word==mf_fp_after_word,187);
#else
 mf_require(mf_lazy_before==1,182);
 mf_require(mf_fp_before_word==0xa5a5a5a5u&&mf_fp_after_word!=mf_fp_before_word,188);
#endif
#else
 mf_require(mf_basic_frames>0&&mf_fp_frames==0,183);
#endif
#ifdef MF_NESTED
 mf_require(mf_nested>0,184);
#endif
 mf_text("MF ticks/pending-requests/completed-switches/nested\n");mf_number(mf_ticks);mf_number(mf_pend_requests);mf_number(mf_switches);mf_number(mf_nested);
 mf_text("MF basic/extended/lazy-before/lazy-after/MSP-min\n");mf_number(mf_basic_frames);mf_number(mf_fp_frames);mf_number(mf_lazy_before);mf_number(mf_lazy_after);mf_number(mf_stack_min);
 mf_text("MF mask observer-counts/body-counts (emulator clock)\n");mf_number(mask_empty);mf_number(mask_body);
 mf_text("MF reserved/written first low-FP frame word\n");mf_number(mf_fp_before_word);mf_number(mf_fp_after_word);
 mf_text("MF painted extension words before/after first FP instruction\n");mf_number(mf_fp_paint_before);mf_number(mf_fp_paint_after);
 mf_text("MF 25MHz emulated counts max-pend-latency/max-ISR/read-overhead/FP-instruction\n");mf_number(mf_pend_latency_max);mf_number(mf_isr_ticks_max);mf_number(mf_clock_read_ticks);mf_number(mf_fp_instruction_ticks);
 mf_text("MF local fault CFSR/PC; peer progress and terminal join/reap\n");mf_number(mf_fault_cfsr);mf_number(mf_fault_pc);
 for(unsigned i=0;i<4;i++){unsigned first=0;while(first<1024&&stacks[i][first]==0xa5a5a5a5u)first++;
  mf_text("MF task/high-water-words\n");mf_number(i+1);mf_number(1024-first);}
 (void)p;mf_text("PASS MF preemptive\n");mf_finish(0);
}
void mf_main(void){mf_port_init();cmx_sem_init(&ready,0,1);
 mf_require(cmx_mutex_init(&mf_machine.kernel,&owned[0],0,0)==CMX_OK&&cmx_mutex_init(&mf_machine.kernel,&owned[1],0,0)==CMX_OK,185);
 for(unsigned i=0;i<2;i++){for(unsigned j=0;j<8;j++)probes[i].integer[j]=0x22000000u+i*0x100u+j;
  for(unsigned j=0;j<16;j++)probes[i].high_fp[j]=0x3f800000u+i*0x10000u+j*32u;
  probes[i].fpscr=i<<22;}
 void(*entries[4])(void)={first,second,broken,supervisor};for(unsigned i=0;i<4;i++)handles[i]=mf_port_add(entries[i],stacks[i],1024);
 mf_port_start();}
