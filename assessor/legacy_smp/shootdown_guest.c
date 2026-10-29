/* Original private mapping target, appended to the completed startup unit. */
#include "shootdown.h"
#include "locks.c"
#include "shootdown.c"
#ifndef STG_GLOBAL
#define STG_GLOBAL 0
#endif
#ifndef STG_MUTANT
#define STG_MUTANT 0
#endif
extern uint64_t boot_pdpt[];
extern void stg_irq(void),stg_fault(void);
extern int stg_try_write(uintptr_t,uint64_t);
extern unsigned char stg_write_site[],stg_after_write[];
static struct st_world stg_world;
static uint64_t stg_pml4[2][512] ALIGN(4096),stg_pdpt[2][512] ALIGN(4096),stg_pd[2][512] ALIGN(4096),stg_pt[2][512] ALIGN(4096);
static unsigned char stg_old[4][4096] ALIGN(4096),stg_new[4][4096] ALIGN(4096);
static unsigned stg_go,stg_ready[4],stg_command[4],stg_done[4],stg_observed[4],stg_faulted[4],stg_probing[4];
static unsigned stg_local[4],stg_interrupts[4],stg_permission,stg_current_space[4],stg_notify_count;
static uint64_t stg_error[4],stg_fault_address[4];
static uintptr_t stg_va(unsigned page){return UINT64_C(0x40000000)+(uintptr_t)page*4096;}
static void stg_fail(const char *reason){text("ST_GUEST_CHECK:");text(reason);text("\n");unsigned code=17;__asm__ volatile("outl %0,$0xf4"::"a"(code));for(;;)__asm__ volatile("cli;hlt");}
static uint64_t stg_ticks(void){unsigned lo,hi;__asm__ volatile("lfence;rdtsc":"=a"(lo),"=d"(hi)::"memory");return (uint64_t)hi<<32|lo;}
static void stg_flush_all(void){uint64_t cr4;__asm__ volatile("mov %%cr4,%0":"=r"(cr4));uint64_t cleared=cr4&~UINT64_C(128);__asm__ volatile("mov %0,%%cr4"::"r"(cleared):"memory");__asm__ volatile("mov %0,%%cr4"::"r"(cr4):"memory");}
static int stg_publish(void *unused,struct st_request r){
 (void)unused;unsigned first=(unsigned)((r.first-stg_va(0))/4096);
 if(r.first<stg_va(0)||first+r.pages>4)stg_fail("publication-range");
 for(unsigned space=0;space<2;space++)if(space==r.space||r.scope==ST_GLOBAL)for(unsigned i=first;i<first+r.pages;i++){
  uint64_t value=stg_permission?(stg_pt[space][i]&~UINT64_C(2)):((uintptr_t)stg_new[i]|3|(STG_GLOBAL?256:0));
  __atomic_store_n(&stg_pt[space][i],value,__ATOMIC_RELEASE);
 }
 __asm__ volatile("mfence":::"memory");return 1;
}
static int stg_invalidate(void *unused,unsigned cpu,struct st_request r){
 (void)unused;if(cpu!=cmk2_cpu_index())stg_fail("nonlocal-invalidation");
 if(r.scope==ST_CONTEXT){stg_flush_all();uintptr_t cr3=(uintptr_t)stg_pml4[r.space];__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");stg_current_space[cpu]=r.space;}
 else for(unsigned i=0;i<r.pages;i++){uintptr_t page=r.first+(uintptr_t)i*4096;__asm__ volatile("invlpg (%0)"::"r"(page):"memory");stg_local[cpu]++;}
 return 1;
}
static int stg_notify(void *unused,struct st_request r,unsigned targets){
 (void)unused;unsigned me=cmk2_cpu_index();stg_notify_count++;
 for(unsigned cpu=0;cpu<topology.count;cpu++)if(targets&(1u<<cpu)){
  if(cpu==me){if(st_handle(&stg_world,cpu,r)!=ST_OK)stg_fail("local-handler-under-membership-lock");}
  else ipi(topology.ids[cpu],246);
 }
 return 1;
}
void stg_interrupt(uint64_t *frame){
 unsigned cpu=cmk2_cpu_index();
 if(frame[15]==14){
  uint64_t address;__asm__ volatile("mov %%cr2,%0":"=r"(address));
  if(!stg_probing[cpu]||frame[17]!=(uintptr_t)stg_write_site||address!=stg_va(0)||(frame[16]&3)!=3)stg_fail("unexpected-page-fault");
  stg_faulted[cpu]++;stg_error[cpu]=frame[16];stg_fault_address[cpu]=address;frame[17]=(uintptr_t)stg_after_write;return;
 }
 if(frame[15]!=246)stg_fail("unexpected-interrupt");
 for(unsigned space=0;space<2;space++){struct st_request r;int result=st_request(&stg_world,space,&r);if(result==ST_OK&&(r.targets&(1u<<cpu))){result=st_handle(&stg_world,cpu,r);if(result!=ST_OK&&result!=ST_STALE&&result!=ST_INCOMPLETE)stg_fail("remote-handler");}}
 stg_interrupts[cpu]++;lapic[0xb0/4]=0;
}
static void stg_install(unsigned cpu){
 uint64_t irq=(uintptr_t)stg_irq,fault=(uintptr_t)stg_fault;
 cpus[cpu].idt[246]=(struct gate){irq&65535,8,2,0x8e,(irq>>16)&65535,(uint32_t)(irq>>32),0};
 cpus[cpu].idt[14]=(struct gate){fault&65535,8,1,0x8e,(fault>>16)&65535,(uint32_t)(fault>>32),0};
 uint64_t cr0,cr4;__asm__ volatile("mov %%cr0,%0":"=r"(cr0));cr0|=UINT64_C(1)<<16;__asm__ volatile("mov %0,%%cr0"::"r"(cr0):"memory");
 __asm__ volatile("mov %%cr4,%0":"=r"(cr4));if(cr4&(UINT64_C(1)<<17))stg_fail("unexpected-PCID");if(STG_GLOBAL)cr4|=128;else cr4&=~UINT64_C(128);__asm__ volatile("mov %0,%%cr4"::"r"(cr4):"memory");
}
static void stg_cache(void){for(unsigned i=0;i<4;i++)if(*(volatile uint64_t *)stg_va(i)!=100+i)stg_fail("initial-cache-value");}
static unsigned stg_write(unsigned cpu){stg_probing[cpu]=1;int result=stg_try_write(stg_va(0),700+cpu);stg_probing[cpu]=0;return (unsigned)result;}
static void stg_worker(unsigned cpu){
 while(!__atomic_load_n(&stg_go,__ATOMIC_ACQUIRE))pause_cpu();stg_install(cpu);
 if(st_enter(&stg_world,cpu,0,NULL)!=ST_OK)stg_fail("initial-space-entry");stg_cache();
 if(cpu!=1)__asm__ volatile("sti":::"memory");__atomic_store_n(&stg_ready[cpu],1,__ATOMIC_RELEASE);
 unsigned previous=0;
 for(;;){unsigned command=__atomic_load_n(&stg_command[cpu],__ATOMIC_ACQUIRE);if(command==previous){pause_cpu();continue;}previous=command;unsigned action=command&255;
  if(action==1){stg_observed[cpu]=(unsigned)*(volatile uint64_t *)stg_va(0);}
  else if(action==2){__asm__ volatile("sti":::"memory");}
  else if(action==3){for(unsigned i=0;i<4;i++)if(*(volatile uint64_t *)stg_va(i)!=(i==0?200u:100u+i))stg_fail("post-reclaim-translation");}
  else if(action==4){stg_flush_all();uintptr_t cr3=(uintptr_t)stg_pml4[0];__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");stg_cache();}
  else if(action==5){if(st_leave(&stg_world,cpu,NULL)!=ST_OK||st_enter(&stg_world,cpu,cpu%2,NULL)!=ST_OK)stg_fail("changed-space-membership");}
  else if(action==6){stg_observed[cpu]=stg_write(cpu);}
  else if(action==99){__atomic_store_n(&stg_done[cpu],command,__ATOMIC_RELEASE);for(;;)__asm__ volatile("cli;hlt");}
  else stg_fail("command");
  __atomic_store_n(&stg_done[cpu],command,__ATOMIC_RELEASE);
 }
}
static void stg_command_all(unsigned command){for(unsigned cpu=1;cpu<topology.count;cpu++)__atomic_store_n(&stg_command[cpu],command,__ATOMIC_RELEASE);for(unsigned cpu=1;cpu<topology.count;cpu++)for(unsigned n=0;__atomic_load_n(&stg_done[cpu],__ATOMIC_ACQUIRE)!=command;n++){if(n==100000000)stg_fail("command-deadline");pause_cpu();}}
static void stg_complete(struct st_request r){struct st_permit permit;for(unsigned n=0;;n++){int result=st_finish(&stg_world,r,0,&permit,NULL);if(result==ST_OK)return;if(result!=ST_INCOMPLETE||n==100000000)stg_fail("completion-deadline");pause_cpu();}}
static void stg_reset(unsigned command){
 for(unsigned space=0;space<2;space++)for(unsigned i=0;i<4;i++){*(uint64_t *)stg_old[i]=100+i;stg_pt[space][i]=(uintptr_t)stg_old[i]|3|(STG_GLOBAL?256:0);}
 __asm__ volatile("mfence":::"memory");stg_command_all(command);stg_flush_all();uintptr_t cr3=(uintptr_t)stg_pml4[0];__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");stg_cache();
}
void stg_bsp(void){
 if(topology.count<2)stg_fail("two-CPUs-required");
 for(unsigned space=0;space<2;space++){stg_pml4[space][0]=(uintptr_t)stg_pdpt[space]|3;for(unsigned i=0;i<4;i++)stg_pdpt[space][i]=boot_pdpt[i];stg_pdpt[space][1]=(uintptr_t)stg_pd[space]|3;stg_pd[space][0]=(uintptr_t)stg_pt[space]|3;for(unsigned i=0;i<4;i++)stg_pt[space][i]=(uintptr_t)stg_old[i]|3|(STG_GLOBAL?256:0);}
 for(unsigned i=0;i<4;i++){*(uint64_t *)stg_old[i]=100+i;*(uint64_t *)stg_new[i]=200+i;}
 if(st_init(&stg_world,topology.count,101,202,(struct st_port){stg_publish,stg_invalidate,stg_notify,NULL})!=ST_OK)stg_fail("world-init");
 stg_install(0);if(st_enter(&stg_world,0,0,NULL)!=ST_OK)stg_fail("BSP-entry");stg_cache();__atomic_store_n(&stg_go,1,__ATOMIC_RELEASE);
 for(unsigned cpu=1;cpu<topology.count;cpu++)while(!__atomic_load_n(&stg_ready[cpu],__ATOMIC_ACQUIRE))pause_cpu();
 struct st_request r;struct st_permit permit;if(st_begin(&stg_world,0,0,stg_va(0),1,STG_GLOBAL?ST_GLOBAL:ST_PAGE,&r,NULL)!=ST_OK)stg_fail("first-request");
 if(st_finish(&stg_world,r,1,&permit,NULL)!=ST_TIMEOUT)stg_fail("timeout-must-retain-frame");
 if(STG_MUTANT==1){*(volatile uint64_t *)stg_old[0]=999;stg_command_all(1);if(stg_observed[1]!=999)stg_fail("early-reuse-control-not-observed");stg_fail("early-reuse-stale-read-999");}
 if(*(uint64_t *)stg_old[0]!=100)stg_fail("retained-frame-changed");stg_command_all(2);stg_complete(r);*(volatile uint64_t *)stg_old[0]=999;stg_command_all(3);
 text("ST_RECLAIM global=");number(STG_GLOBAL);text(" timeout_retained=1 new_marker=999 old_va_after_ack=200\n");
 for(unsigned condition=0;condition<2;condition++){
  stg_reset(((condition+1)<<8)|4);unsigned requests=stg_notify_count;uint64_t begin=stg_ticks();
  if(condition==0){if(st_begin(&stg_world,0,0,stg_va(0),4,STG_GLOBAL?ST_GLOBAL:ST_PAGE,&r,NULL)!=ST_OK)stg_fail("batch-request");stg_complete(r);}
  else for(unsigned page=0;page<4;page++){if(st_begin(&stg_world,0,0,stg_va(page),1,STG_GLOBAL?ST_GLOBAL:ST_PAGE,&r,NULL)!=ST_OK)stg_fail("single-request");stg_complete(r);}
  text("ST_BATCH condition=");number(condition);text(" pages=4 requests=");number(stg_notify_count-requests);text(" sender_ticks=");ss_u64(stg_ticks()-begin);text("\n");
 }
 /* Two actual CR3 roots share frames but have independent non-global R/W. */
 stg_permission=1;for(unsigned space=0;space<2;space++)for(unsigned i=0;i<4;i++)stg_pt[space][i]=(uintptr_t)stg_old[i]|3;__asm__ volatile("mfence":::"memory");stg_command_all(5);stg_flush_all();
 if(st_begin(&stg_world,0,0,stg_va(0),1,ST_PAGE,&r,NULL)!=ST_OK)stg_fail("permission-A");unsigned target_a=r.targets;stg_complete(r);stg_observed[0]=stg_write(0);stg_command_all(6);
 for(unsigned cpu=0;cpu<topology.count;cpu++)if(stg_observed[cpu]!=(cpu%2))stg_fail("wrong-space-permission-boundary");
 if(st_begin(&stg_world,0,1,stg_va(0),1,ST_PAGE,&r,NULL)!=ST_OK)stg_fail("permission-B");unsigned target_b=r.targets;stg_complete(r);stg_observed[0]=stg_write(0);stg_command_all(262);
 for(unsigned cpu=0;cpu<topology.count;cpu++)if(stg_observed[cpu]||!stg_faulted[cpu]||stg_fault_address[cpu]!=stg_va(0))stg_fail("permission-revocation-not-enforced");
 stg_command_all(99);
 text("ST_PERMISSION targets_a=");number(target_a);text(" targets_b=");number(target_b);text(" shared_frame_retained=1\n");
 for(unsigned cpu=0;cpu<topology.count;cpu++){text("ST_CPU cpu=");number(cpu);text(" invalidated_pages=");number(stg_local[cpu]);text(" interrupts=");number(stg_interrupts[cpu]);text(" faults=");number(stg_faulted[cpu]);text(" error=");ss_u64(stg_error[cpu]);text("\n");}
 text("ST_GUEST_PASS:actual-invalidation-reclaim-permission cpus=");number(topology.count);text(" global=");number(STG_GLOBAL);text("\n");unsigned code=16;__asm__ volatile("outl %0,$0xf4"::"a"(code));for(;;)__asm__ volatile("cli;hlt");
}
