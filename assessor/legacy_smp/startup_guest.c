/* Appended to a private copy of retained smp.c, with its old entry renamed.
 * Uses its actual discovery, per-CPU tables, APIC and PIT driver unchanged. */
#include "startup.h"
#include "startup.c"
#ifndef SS_BASE
#define SS_BASE 0x8000
#endif
#ifndef SS_MODE
#define SS_MODE 0
#endif
#ifndef SS_GUEST_MUTANT
#define SS_GUEST_MUTANT 0
#endif
extern unsigned char ss_ap_0_start[],ss_ap_0_end[],ss_ap_1_start[],ss_ap_1_end[],ss_ap_2_start[],ss_ap_2_end[];
extern void ss_park(unsigned,uintptr_t) __attribute__((noreturn));
static unsigned char ss_boot[CMK2_MAX_CPUS][4096] ALIGN(16);
static struct {unsigned generation,apic,verified,ack,quiescent;uint64_t initial_stack,page;} ss_observed[CMK2_MAX_CPUS];
static unsigned ss_delay_release;
static struct ss_world ss_world;
static struct ss_ticket ss_tickets[CMK2_MAX_CPUS];
static void ss_fail(const char *reason){text("SS_GUEST_CHECK:");text(reason);text("\n");unsigned value=17;__asm__ volatile("outl %0,$0xf4"::"a"(value));for(;;)__asm__ volatile("cli;hlt");}
static void ss_u64(uint64_t v){number((unsigned)(v>>32));text(":");number((unsigned)v);}
void ss_parked(unsigned index){
 __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);
 for(;;)__asm__ volatile("cli;hlt");
}
static void ss_entry(unsigned index,unsigned generation,unsigned expected_apic,unsigned page,uintptr_t initial_stack){
 if(index<1||index>=topology.count)ss_fail("bad-index");
 if(SS_GUEST_MUTANT==1)for(;;)pause_cpu();
 if(SS_MODE==2&&index==1)while(!__atomic_load_n(&ss_delay_release,__ATOMIC_ACQUIRE))pause_cpu();
 uintptr_t current_stack;uint64_t cr0,cr3,cr4;unsigned low,high;__asm__ volatile("mov %%rsp,%0":"=r"(current_stack));
 __asm__ volatile("mov %%cr0,%0":"=r"(cr0));__asm__ volatile("mov %%cr3,%0":"=r"(cr3));__asm__ volatile("mov %%cr4,%0":"=r"(cr4));
 __asm__ volatile("rdmsr":"=a"(low),"=d"(high):"c"(0xc0000080));
 unsigned verified=0;
 if((cr0&UINT64_C(0x80010001))==UINT64_C(0x80010001)&&(cr4&0x20)&&(low&0xd00)==0xd00&&high==0)verified|=1;
 if(cr3==(uintptr_t)boot_pml4)verified|=2;
 uintptr_t begin=(uintptr_t)ss_boot[index],end=begin+sizeof ss_boot[index];
 if(current_stack>=begin&&current_stack<end&&initial_stack==end-8&&((initial_stack+8)&15)==0)verified|=4;
 unsigned actual=lapic[0x20/4]>>24;if(actual==expected_apic&&cmk2_cpu_index()==index)verified|=8;
 local_tables(index);
 ss_observed[index].generation=SS_GUEST_MUTANT==2?generation-1:generation;ss_observed[index].apic=actual;ss_observed[index].verified=verified;ss_observed[index].initial_stack=initial_stack;ss_observed[index].page=page;
 __atomic_store_n(&ss_observed[index].ack,1,__ATOMIC_RELEASE);
 ss_park(index,(uintptr_t)(cpus[index].boot+sizeof cpus[index].boot));
}
static void ss_launch(unsigned index){
 unsigned slot=SS_MODE==1?index-1:0;uint64_t page=SS_BASE+4096*slot;struct ss_ticket ticket;
 if(ss_begin(&ss_world,slot,index,topology.ids[index],page,(uintptr_t)ss_boot[index],(uintptr_t)(ss_boot[index]+sizeof ss_boot[index]),&ticket)!=SS_OK)ss_fail("resource-admission");ss_tickets[index]=ticket;
 unsigned char *begins[]={ss_ap_0_start,ss_ap_1_start,ss_ap_2_start},*ends[]={ss_ap_0_end,ss_ap_1_end,ss_ap_2_end};size_t n=(size_t)(ends[slot]-begins[slot]);if(n>0xf00)ss_fail("trampoline-size");memcpy((void *)(uintptr_t)page,begins[slot],n);
 volatile uint64_t *m=(volatile uint64_t *)(uintptr_t)(page+0xf00);m[0]=(uintptr_t)boot_pml4;m[1]=(uintptr_t)(ss_boot[index]+sizeof ss_boot[index]);m[2]=(uintptr_t)ss_entry;m[3]=index;m[4]=ticket.generation;m[5]=topology.ids[index];__atomic_thread_fence(__ATOMIC_SEQ_CST);
 ipi(topology.ids[index],0xc500);delay();ipi(topology.ids[index],0x8500);delay();ipi(topology.ids[index],0x600|(unsigned)(page>>12));
}
static void ss_collect(unsigned index){
 struct ss_ticket t=ss_tickets[index];unsigned timed_out=0;
 for(unsigned spin=0;!__atomic_load_n(&ss_observed[index].ack,__ATOMIC_ACQUIRE);++spin){
  if(spin==10000000){
   if(SS_MODE!=2||index!=1||timed_out)ss_fail("missing-ack");
   if(ss_timeout(&ss_world,t)!=SS_OK||ss_release(&ss_world,t)!=SS_NOT_QUIESCENT)ss_fail("timeout-reuse");
   struct ss_ticket untouched={99,99};if(ss_begin(&ss_world,t.slot,index,topology.ids[index],SS_BASE,(uintptr_t)ss_boot[index],(uintptr_t)(ss_boot[index]+sizeof ss_boot[index]),&untouched)!=SS_BUSY||untouched.slot!=99)ss_fail("timeout-new-attempt");
   text("SS_TIMEOUT_RETAIN:index=");number(index);text(" generation=");number(t.generation);text("\n");timed_out=1;spin=0;__atomic_store_n(&ss_delay_release,1,__ATOMIC_RELEASE);
  }pause_cpu();
 }
 struct ss_ticket reported={t.slot,ss_observed[index].generation};struct ss_evidence evidence={ss_observed[index].apic,ss_observed[index].verified,ss_observed[index].initial_stack};int result=ss_ack(&ss_world,reported,evidence);
 if(result!=(timed_out?SS_LATE:SS_OK))ss_fail("stale-or-incomplete-ack");
 if(ss_ack(&ss_world,reported,evidence)!=SS_DUPLICATE)ss_fail("duplicate-ack");
 for(unsigned spin=0;!__atomic_load_n(&ss_observed[index].quiescent,__ATOMIC_ACQUIRE);++spin){if(spin==10000000)ss_fail("quiescence-missing");pause_cpu();}
 /* The observed handoff occurs on the permanent per-CPU stack. The abandoned
    bootstrap stack, code page and mailbox are no longer reachable by that AP. */
 if(ss_quiescent(&ss_world,t,1)!=SS_OK||ss_release(&ss_world,t)!=SS_OK)ss_fail("quiescence-release");
 text("SS_CPU index=");number(index);text(" apic=");number(ss_observed[index].apic);text(" generation=");number(reported.generation);text(" verified=");number(evidence.verified);text(" page=");ss_u64(ss_observed[index].page);text(" bootstrap_entry=");ss_u64(evidence.entry_stack);text(" permanent_top=");ss_u64((uintptr_t)(cpus[index].boot+sizeof cpus[index].boot));text(" late=");number(timed_out);text("\n");
}
void cmk2_smp_run(unsigned info){
 discover(info);unsigned bsp=cmk2_cpu_index();if(bsp){unsigned char id=topology.ids[0];topology.ids[0]=topology.ids[bsp];topology.ids[bsp]=id;}
 outb(0x21,255);outb(0xa1,255);local_tables(0);ss_world.parallel=SS_MODE==1;
 text("SS_BSP cpus=");number(topology.count);text(" apic=");number(topology.ids[0]);text(" mode=");number(SS_MODE);text(" base=");number(SS_BASE);text("\n");
 for(unsigned index=1;index<topology.count;++index){ss_launch(index);if(SS_MODE!=1)ss_collect(index);}
 if(SS_MODE==1)for(unsigned index=1;index<topology.count;++index)ss_collect(index);
 text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\n");unsigned value=16;__asm__ volatile("outl %0,$0xf4"::"a"(value));for(;;)__asm__ volatile("cli;hlt");
}
