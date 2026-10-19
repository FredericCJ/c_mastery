#include "port.h"
Mf mf_machine;
volatile uint32_t mf_ticks,mf_pend_requests,mf_switches,mf_nested;
volatile uint32_t mf_basic_frames,mf_fp_frames,mf_lazy_before,mf_lazy_after;
volatile uint32_t mf_handler_alignment,mf_stack_min;
volatile uint32_t mf_fp_before_word,mf_fp_after_word;
volatile uint32_t mf_pend_latency_max,mf_isr_ticks_max,mf_fp_instruction_ticks,mf_clock_read_ticks;
volatile uint32_t mf_fp_paint_before,mf_fp_paint_after;
volatile uint32_t mf_fault_count,mf_fault_pc,mf_fault_cfsr;
static uint32_t idle_stack[256] __attribute__((aligned(8)));
static unsigned started;
static uint32_t pending_start;
static unsigned pending_active;
static uint32_t clock_count(void){return UINT32_MAX-MF_REG(0x40000004);}
/* NMI owns writes; SysTick consumes only after NMI return. No scheduler
 * operation is permitted in the urgent producer. One outstanding record. */
static volatile uint32_t urgent_payload,urgent_sequence;
static uint32_t urgent_consumed;
uint32_t mf_lock(void){uint32_t old;__asm__ volatile("mrs %0,primask\ncpsid i":"=r"(old)::"memory");return old;}
void mf_unlock(uint32_t old){__asm__ volatile("msr primask,%0\ndsb\nisb"::"r"(old):"memory");}
void mf_pend(void){if(!(MF_REG(0xe000ed04)&(1u<<28))){pending_start=clock_count();pending_active=1;}
 mf_pend_requests++;MF_REG(0xe000ed04)=1u<<28;__asm__ volatile("dsb\nisb":::"memory");}
static void idle(void){for(;;)__asm__ volatile("wfi");}
void mf_task_return(void){unsigned old=mf_lock();
 mf_require(mf_exit(&mf_machine,mf_handle(&mf_machine,(unsigned)mf_machine.executing),0)==CMX_OK,101);
 mf_pend();mf_unlock(old);for(;;)__asm__ volatile("wfi");}
void mf_port_init(void){
 mf_require(mf_init(&mf_machine,idle_stack,256,CMX_RR)==CMX_OK,102);
 mf_machine.sp[0][15]=(uint32_t)(uintptr_t)idle;mf_machine.sp[0][14]=(uint32_t)(uintptr_t)mf_task_return;
 MF_REG(0xe000ed20)=(MF_REG(0xe000ed20)&0x0000ffffu)|0x80ff0000u;
 MF_REG(0x40000000)=0;MF_REG(0x40000008)=UINT32_MAX;MF_REG(0x40000000)=1;
 MF_REG(0xe000ed14)|=1u<<9;MF_REG(0xe000ed24)|=(1u<<16)|(1u<<18);
 MF_REG(0xe000ef34)=(1u<<31)
#ifndef MF_EAGER
 |(1u<<30)
#endif
 ;mf_stack_min=UINT32_MAX;
}
MfHandle mf_port_add(void(*entry)(void),uint32_t *stack,size_t words){MfHandle h=0;
 unsigned old=mf_lock();enum cmx_result r=mf_add(&mf_machine,stack,words,(uint32_t)(uintptr_t)entry,(uint32_t)(uintptr_t)mf_task_return,1,0,2,&h);mf_unlock(old);
 mf_require(r==CMX_OK,103);return h;}
uint32_t *mf_choose(uint32_t *old){
 if(pending_active){uint32_t elapsed=clock_count()-pending_start;if(elapsed>mf_pend_latency_max)mf_pend_latency_max=elapsed;pending_active=0;}
 uintptr_t msp;__asm__ volatile("mrs %0,msp":"=r"(msp));mf_require(!(msp&7u),104);mf_handler_alignment++;
 if(old&&mf_machine.executing>=0){unsigned t=(unsigned)mf_machine.executing;cmx_task *task=&mf_machine.kernel.task[t];
#ifdef MF_BAD_STACK_BOUND
  old=(uint32_t *)(task->stack_low-4u);
#endif
  mf_require((uintptr_t)old>=task->stack_low+32&&(uintptr_t)old<task->stack_high,105);
  mf_require(*(uint32_t*)task->stack_low==0xa5a5a5a5u,106);mf_machine.sp[t]=old;
 }
 int n=mf_schedule(&mf_machine);mf_require(n>=0,107);
 /* Still on the separate MSP. No task/reaper can run until exception return. */
 mf_machine.executing=n;mf_switches++;return mf_machine.sp[n];
}
static unsigned may_wait(void){uint32_t ip,pm,bp,fm;__asm__ volatile("mrs %0,ipsr\nmrs %1,primask\nmrs %2,basepri\nmrs %3,faultmask":"=r"(ip),"=r"(pm),"=r"(bp),"=r"(fm));return started&&!ip&&!pm&&!bp&&!fm;}
enum cmx_result mf_take(cmx_sem *s,cmx_deadline d){if(!may_wait())return CMX_INVALID;unsigned p=mf_lock(),t=(unsigned)mf_machine.executing;enum cmx_result r=cmx_sem_take(&mf_machine.kernel,s,t,d);if(r==CMX_BLOCKED)mf_pend();mf_unlock(p);return r==CMX_BLOCKED?mf_machine.kernel.task[t].result:r;}
enum cmx_result mf_give(cmx_sem *s){unsigned p=mf_lock();enum cmx_result r=cmx_sem_give(&mf_machine.kernel,s);mf_unlock(p);return r;}
unsigned mf_mask_enter(unsigned request){unsigned old,next;__asm__ volatile("mrs %0,basepri":"=r"(old));mf_require(mf_basepri(old,request,4,&next)==CMX_OK,108);__asm__ volatile("msr basepri,%0\ndsb\nisb"::"r"(next):"memory");return old;}
void mf_mask_leave(unsigned old){
#ifdef MF_BAD_MASK
 old=0;
#endif
 __asm__ volatile("msr basepri,%0\ndsb\nisb"::"r"(old):"memory");}
void mf_nmi(uint32_t exc,uint32_t msp){mf_require(!(msp&7u),109);mf_require((exc&8u)==0,110);mf_nested++;
 mf_require(urgent_sequence==urgent_consumed,116);urgent_payload=mf_nested^0xa55a1234u;urgent_sequence++;
#if MF_FP
 __asm__ volatile("vmov.f32 s0,#1.0":::"s0");
#endif
}
void mf_tick(uint32_t exc,uint32_t *frame,uint32_t msp){
 uint32_t isr_start=clock_count();
 mf_require(!(msp&7u),111);if(msp<mf_stack_min)mf_stack_min=msp;
 if(exc&16u){mf_basic_frames++;mf_require((frame[7]&(1u<<24))!=0,112);}else{
  mf_fp_frames++;
#ifdef MF_FRAME_DUMP
  if(mf_fp_frames==1){mf_text("MF raw extended frame from PSP\n");for(unsigned i=0;i<26;i++)mf_number(frame[i]);}
#endif
  /* Hardware layout is inspected by both the xPSR and Thumb-state predicate;
   * record it independently of a drawing's address orientation. */
  mf_require((frame[7]&(1u<<24))!=0,113);
  mf_lazy_before|=MF_REG(0xe000ef34)&1u;
  if(mf_fp_frames==1){mf_fp_before_word=frame[8];for(unsigned i=8;i<26;i++)mf_fp_paint_before+=frame[i]==0xa5a5a5a5u;}
  uint32_t read_start=clock_count(),read_end=clock_count();
  uint32_t instruction_start=clock_count();
#if MF_FP
  __asm__ volatile("vmov.f32 s0,#2.0":::"s0","memory");
#endif
  uint32_t instruction_elapsed=clock_count()-instruction_start;
  if(mf_fp_frames==1){mf_clock_read_ticks=read_end-read_start;mf_fp_instruction_ticks=instruction_elapsed;}
  mf_lazy_after|=MF_REG(0xe000ef34)&1u;
  if(mf_fp_frames==1){mf_fp_after_word=frame[8];for(unsigned i=8;i<26;i++)mf_fp_paint_after+=frame[i]==0xa5a5a5a5u;}
 }
 mf_ticks++;cmx_tick(&mf_machine.kernel,mf_ticks);
 mf_tick_hook();
#ifdef MF_NESTED
 MF_REG(0xe000ed04)=1u<<31;__asm__ volatile("dsb\nisb":::"memory");
 mf_require(urgent_sequence==urgent_consumed+1&&urgent_payload==(mf_nested^0xa55a1234u),117);urgent_consumed++;
#endif
#ifdef MF_LONG_ISR
 for(volatile unsigned delay=0;delay<300;delay++)__asm__ volatile("nop");
#endif
 mf_pend();
 uint32_t isr_elapsed=clock_count()-isr_start;if(isr_elapsed>mf_isr_ticks_max)mf_isr_ticks_max=isr_elapsed;
}
static void abandoned(void){for(;;){mf_pend();__asm__ volatile("wfi");}}
void mf_local_fault(uint32_t *frame,uint32_t exc){
 unsigned p=mf_lock();mf_require((exc&12u)==12u&&mf_machine.executing>0,114);
 /* Basic r0..xPSR occupy words0..7 in both frame forms on this Armv7-M
  * target; the18-word low-FP extension follows them. The raw-frame pilot
  * and forced low-FP checks bind this decoder to the actual machine. */
 uint32_t *basic=frame;uint32_t code=MF_REG(0xe000ed28);
 mf_require((code&((1u<<3)|(1u<<4)|(1u<<5)|(1u<<11)|(1u<<12)|(1u<<13)))==0,115);
 mf_fault_count++;mf_fault_pc=basic[6];mf_fault_cfsr=code;
 enum cmx_result r=mf_fault(&mf_machine,mf_handle(&mf_machine,(unsigned)mf_machine.executing),code,basic[6]);
 if(r!=CMX_OK){mf_text("MF unsafe owner fault retained resources\n");
  for(unsigned i=0;i<mf_machine.kernel.mutex_count;i++)mf_number((uint32_t)mf_machine.kernel.mutex[i]->owner);
  mf_finish(3);}
 MF_REG(0xe000ed28)=code;MF_REG(0xe000ed94)=0;__asm__ volatile("dsb\nisb":::"memory");
 basic[6]=(uint32_t)(uintptr_t)abandoned;mf_pend();mf_unlock(p);
}
void mf_port_start(void){started=1;MF_REG(0xe000e014)=9999;MF_REG(0xe000e018)=0;MF_REG(0xe000e010)=7;
 __asm__ volatile("cpsie i\nsvc 0":::"memory");for(;;)__asm__ volatile("wfi");}
