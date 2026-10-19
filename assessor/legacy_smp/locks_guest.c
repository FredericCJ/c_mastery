#include "locks.h"
#include "locks.c"
#ifndef SL_KIND
#define SL_KIND 0
#endif
#ifndef SL_GUEST_MUTANT
#define SL_GUEST_MUTANT 0
#endif
extern void sl_probe_irq(void);
static SlLock sl_outer,sl_inner;
static SlContext sl_contexts[CMK2_MAX_CPUS];
static SlNode sl_nodes[CMK2_MAX_CPUS][2];
static SlStats sl_task_stats[CMK2_MAX_CPUS],sl_irq_stats[CMK2_MAX_CPUS];
static unsigned sl_go,sl_begin,sl_ready[CMK2_MAX_CPUS],sl_done[CMK2_MAX_CPUS],sl_irqs[CMK2_MAX_CPUS],sl_owns[CMK2_MAX_CPUS],sl_remote;
static uint64_t sl_work;
static void sl_fail(const char *reason){text("SL_GUEST_CHECK:");text(reason);text("\n");unsigned value=17;__asm__ volatile("outl %0,$0xf4"::"a"(value));for(;;)__asm__ volatile("cli;hlt");}
static unsigned sl_enabled(void){uint64_t flags;__asm__ volatile("pushfq;pop %0":"=r"(flags)::"memory");return (unsigned)((flags>>9)&1);}
static uint64_t sl_save(void *unused){(void)unused;uint64_t flags;__asm__ volatile("pushfq;pop %0;cli":"=r"(flags)::"memory");return (flags>>9)&1;}
static void sl_restore(void *unused,uint64_t saved){(void)unused;if(SL_GUEST_MUTANT==1||saved)__asm__ volatile("sti":::"memory");else __asm__ volatile("cli":::"memory");}
static SlPort sl_port={sl_save,sl_restore,0};
static void sl_self_ipi(void){__asm__ volatile("mfence":::"memory");lapic[0x300/4]=0x44000|244;}
void sl_interrupt(uint64_t *frame) {
 unsigned index=cmk2_cpu_index();if(frame[15]!=244)sl_fail("unexpected-vector");
 if(sl_contexts[index].depth||__atomic_load_n(&sl_owns[index],__ATOMIC_RELAXED))sl_fail("local-handler-waits-for-interrupted-owner");
 SlIrqGuard outer={0},inner={0};
 if(sl_irq_enter(&sl_contexts[index],sl_port,&sl_outer,&sl_nodes[index][0],&outer,&sl_irq_stats[index])!=SL_OK||sl_irq_enter(&sl_contexts[index],sl_port,&sl_inner,&sl_nodes[index][1],&inner,&sl_irq_stats[index])!=SL_OK)sl_fail("ISR-lock-entry");
 sl_work++;
 if(sl_irq_leave(&sl_contexts[index],sl_port,&inner,&sl_irq_stats[index])!=SL_OK||sl_enabled()||sl_irq_leave(&sl_contexts[index],sl_port,&outer,&sl_irq_stats[index])!=SL_OK||sl_enabled())sl_fail("ISR-state-restore");
 lapic[0xb0/4]=0;__atomic_add_fetch(&sl_irqs[index],1,__ATOMIC_RELEASE);
}
static void sl_probe_saved(unsigned index) {
 for(unsigned initial=0;initial<2;initial++) {
  if(initial)__asm__ volatile("sti":::"memory");else __asm__ volatile("cli":::"memory");
  SlIrqGuard outer={0},inner={0};
  if(sl_irq_enter(&sl_contexts[index],sl_port,&sl_outer,&sl_nodes[index][0],&outer,&sl_task_stats[index])!=SL_OK||sl_enabled())sl_fail("IRQ-entry-not-masked");
  if(outer.saved!=initial||sl_irq_enter(&sl_contexts[index],sl_port,&sl_inner,&sl_nodes[index][1],&inner,&sl_task_stats[index])!=SL_OK||inner.saved)sl_fail("nested-saved-state");
  if(sl_irq_leave(&sl_contexts[index],sl_port,&inner,&sl_task_stats[index])!=SL_OK||sl_enabled())sl_fail("IRQ-state-not-restored");
  if(sl_irq_leave(&sl_contexts[index],sl_port,&outer,&sl_task_stats[index])!=SL_OK||sl_enabled()!=initial)sl_fail("outer-state-not-restored");
 }
}
static void sl_local(unsigned index) {
 uint64_t address=(uintptr_t)sl_probe_irq;cpus[index].idt[244]=(struct gate){address&65535,8,1,0x8e,(address>>16)&65535,(uint32_t)(address>>32),0};
 sl_probe_saved(index);__asm__ volatile("cli":::"memory");__atomic_store_n(&sl_ready[index],1,__ATOMIC_RELEASE);
}
static void sl_jobs(unsigned index) {
 __asm__ volatile("sti":::"memory");
 if(SL_GUEST_MUTANT==2&&index==0) {
  SlGuard guard={0};if(sl_enter(&sl_outer,&sl_nodes[index][0],&guard,&sl_task_stats[index])!=SL_OK)sl_fail("raw-probe-entry");
  __atomic_store_n(&sl_owns[index],1,__ATOMIC_RELAXED);sl_self_ipi();
  for(unsigned spin=0;spin<100000000;spin++)pause_cpu();sl_fail("raw-handler-not-delivered");
 }
 for(unsigned iteration=0;iteration<200;iteration++) {
  unsigned initial=iteration&1;if(initial)__asm__ volatile("sti":::"memory");else __asm__ volatile("cli":::"memory");
  SlIrqGuard outer={0},inner={0};
  if(sl_irq_enter(&sl_contexts[index],sl_port,&sl_outer,&sl_nodes[index][0],&outer,&sl_task_stats[index])!=SL_OK||sl_irq_enter(&sl_contexts[index],sl_port,&sl_inner,&sl_nodes[index][1],&inner,&sl_task_stats[index])!=SL_OK)sl_fail("task-entry");
  __atomic_store_n(&sl_owns[index],1,__ATOMIC_RELAXED);sl_work++;sl_self_ipi();
  if(__atomic_load_n(&sl_irqs[index],__ATOMIC_ACQUIRE)!=iteration)sl_fail("masked-IPI-delivered");
  if(sl_irq_leave(&sl_contexts[index],sl_port,&inner,&sl_task_stats[index])!=SL_OK||sl_enabled())sl_fail("inner-restore");
  __atomic_store_n(&sl_owns[index],0,__ATOMIC_RELAXED);
  if(sl_irq_leave(&sl_contexts[index],sl_port,&outer,&sl_task_stats[index])!=SL_OK||sl_enabled()!=initial)sl_fail("task-restore");
  if(!initial){if(__atomic_load_n(&sl_irqs[index],__ATOMIC_ACQUIRE)!=iteration)sl_fail("disabled-outer-restored-enabled");__asm__ volatile("sti":::"memory");}
  for(unsigned spin=0;__atomic_load_n(&sl_irqs[index],__ATOMIC_ACQUIRE)!=iteration+1;spin++){if(spin==100000000)sl_fail("deferred-IPI-missing");pause_cpu();}
 }
 __asm__ volatile("cli":::"memory");__atomic_store_n(&sl_done[index],1,__ATOMIC_RELEASE);
}
static void sl_worker(unsigned index) {
 while(!__atomic_load_n(&sl_go,__ATOMIC_ACQUIRE))pause_cpu();sl_local(index);
 if(index==1){while(!__atomic_load_n(&sl_begin,__ATOMIC_ACQUIRE))pause_cpu();__atomic_store_n(&sl_remote,1,__ATOMIC_RELEASE);}
 while(__atomic_load_n(&sl_begin,__ATOMIC_ACQUIRE)!=2)pause_cpu();sl_jobs(index);
 for(;;)__asm__ volatile("cli;hlt");
}
void sl_bsp(void) {
 if(sl_init(&sl_outer,SL_KIND,1,SL_KIND==SL_TICKET?14:0)!=SL_OK||sl_init(&sl_inner,SL_KIND,2,SL_KIND==SL_TICKET?14:0)!=SL_OK)sl_fail("lock-init");
 for(unsigned i=0;i<topology.count;i++)for(unsigned j=0;j<2;j++)sl_node_init(&sl_nodes[i][j]);
 __atomic_store_n(&sl_go,1,__ATOMIC_RELEASE);sl_local(0);
 for(unsigned i=0;i<topology.count;i++)for(unsigned spin=0;!__atomic_load_n(&sl_ready[i],__ATOMIC_ACQUIRE);spin++){if(spin==100000000)sl_fail("CPU-ready");pause_cpu();}
 __atomic_store_n(&sl_begin,1,__ATOMIC_RELEASE);
 if(topology.count>1){while(!__atomic_load_n(&sl_remote,__ATOMIC_ACQUIRE))pause_cpu();if(sl_enabled())sl_fail("local-mask-premise");text("SL_REMOTE_PROGRESS masked_cpu=0 remote_cpu=1\n");}
 __atomic_store_n(&sl_begin,2,__ATOMIC_RELEASE);sl_jobs(0);
 for(unsigned i=1;i<topology.count;i++)for(unsigned spin=0;!__atomic_load_n(&sl_done[i],__ATOMIC_ACQUIRE);spin++){if(spin==100000000)sl_fail("CPU-completion");pause_cpu();}
 if(sl_work!=400u*topology.count)sl_fail("protected-work");
 for(unsigned i=0;i<topology.count;i++) {
  if(sl_contexts[i].depth||atomic_load(&sl_nodes[i][0].active)||atomic_load(&sl_nodes[i][1].active)||sl_irqs[i]!=200)sl_fail("node-or-context-retirement");
  text("SL_CPU index=");number(i);text(" apic=");number(topology.ids[i]);text(" kind=");number(SL_KIND);text(" tasks=200 irqs=");number(sl_irqs[i]);text(" saved_off=1 saved_on=1 depth=0 nodes=0\n");
 }
 text("SL_GUEST_PASS:IRQ-state-SMP-exclusion-node-lifetime work=");number((unsigned)sl_work);text("\n");unsigned value=16;__asm__ volatile("outl %0,$0xf4"::"a"(value));for(;;)__asm__ volatile("cli;hlt");
}
