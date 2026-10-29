/* Appended after the explicitly imported completed startup adapter. */
#include "percpu.h"
#include "percpu.c"
#ifndef SP_SIMD
#define SP_SIMD 0
#endif
#ifndef SP_GUEST_MUTANT
#define SP_GUEST_MUTANT 0
#endif
extern void sp_probe_irq(void),sp_ud_irq(void),sp_gp_irq(void),sp_raise_ud(void),sp_raise_gp(void);
extern unsigned char sp_ud_site[],sp_ud_return[],sp_gp_site[],sp_gp_return[];
extern void sp_switch(uint64_t **,uint64_t *,void *,void *);
extern int sp_register_probe(uint64_t);
static struct {
 unsigned ready,done,current,nesting,irq,ud,gp,progress;
 uint64_t *idle,*task_sp[2];
 uint64_t irq_frame,ud_frame,gp_frame,peer_before,peer_after;
 unsigned task_done[2];
 unsigned char idle_fx[512] ALIGN(16),task_fx[2][512] ALIGN(16);
 unsigned char task_stack[2][8192] ALIGN(16),emergency[4096] ALIGN(16);
} sp_local[CMK2_MAX_CPUS] ALIGN(64);
static unsigned sp_go,sp_fault_done;
static unsigned sp_fault_cpu(void){return topology.count>1?1:0;}
static void sp_fail(const char *s){text("SP_GUEST_CHECK:");text(s);text("\n");unsigned v=17;__asm__ volatile("outl %0,$0xf4"::"a"(v));for(;;)__asm__ volatile("cli;hlt");}
static unsigned sp_peer_progress(unsigned c){unsigned n=0;for(unsigned i=0;i<topology.count;++i)if(i!=c)n+=__atomic_load_n(&sp_local[i].progress,__ATOMIC_ACQUIRE);return n;}
static void sp_gate(unsigned c,unsigned vector,void (*entry)(void),unsigned ist){uint64_t a=(uintptr_t)entry;cpus[c].idt[vector]=(struct gate){a&65535,8,(uint8_t)ist,0x8e,(a>>16)&65535,(uint32_t)(a>>32),0};}
static void sp_inside(uintptr_t p,size_t bytes,const unsigned char *begin,size_t size){uintptr_t low=(uintptr_t)begin,high=low+size;if(p<low+32||p>high||bytes>high-p)sp_fail("stack-owner-or-extent");}
static void sp_local_init(unsigned c){
 /* Startup already installed this CPU's private GDT/TSS/IDT. */
 cpus[c].tss.rsp[0]=(uintptr_t)(cpus[c].boot+sizeof cpus[c].boot);
 cpus[c].tss.ist[2]=(uintptr_t)(sp_local[c].emergency+sizeof sp_local[c].emergency);
 if(SP_GUEST_MUTANT==1&&c==1)cpus[c].tss.ist[0]=(uintptr_t)(cpus[0].irq+sizeof cpus[0].irq);
 sp_gate(c,244,sp_probe_irq,1);sp_gate(c,6,sp_ud_irq,2);sp_gate(c,13,sp_gp_irq,3);
 for(unsigned i=0;i<32;++i){cpus[c].irq[i]=0x91;cpus[c].tlb_stack[i]=0xa2;sp_local[c].emergency[i]=0xb3;for(unsigned t=0;t<2;++t)sp_local[c].task_stack[t][i]=(unsigned char)(0xc0+c*2+t);}
 struct dt actual_gd,actual_id;unsigned short tr;__asm__ volatile("sgdt %0":"=m"(actual_gd));__asm__ volatile("sidt %0":"=m"(actual_id));__asm__ volatile("str %0":"=r"(tr));
 if(actual_gd.base!=(uintptr_t)cpus[c].gdt||actual_id.base!=(uintptr_t)cpus[c].idt||tr!=40)sp_fail("descriptor-identity");
#if SP_SIMD
 unsigned a=1,b,d,e;__asm__ volatile("cpuid":"+a"(a),"=b"(b),"=c"(e),"=d"(d));if((d&0x07000000)!=0x07000000)sp_fail("fxsr-sse2-unavailable");
 uint64_t cr0,cr4;__asm__ volatile("mov %%cr0,%0":"=r"(cr0));cr0=(cr0&~UINT64_C(12))|2;__asm__ volatile("mov %0,%%cr0"::"r"(cr0));__asm__ volatile("mov %%cr4,%0":"=r"(cr4));cr4|=0x600;__asm__ volatile("mov %0,%%cr4"::"r"(cr4));unsigned mx=0x1f80;__asm__ volatile("fninit;ldmxcsr %1;fxsave64 %0":"=m"(sp_local[c].idle_fx):"m"(mx):"memory");
 memcpy(sp_local[c].task_fx[0],sp_local[c].idle_fx,512);memcpy(sp_local[c].task_fx[1],sp_local[c].idle_fx,512);
#endif
}
void sp_interrupt(uint64_t *f){
 unsigned c=cmk2_cpu_index(),vector=(unsigned)f[15];sp_local[c].nesting++;
 if(vector==244){
  sp_inside((uintptr_t)f,22*8,cpus[c].irq,sizeof cpus[c].irq);sp_inside(f[20],1,sp_local[c].task_stack[sp_local[c].current],8192);
  if(sp_local[c].nesting!=1)sp_fail("irq-nesting");sp_local[c].irq++;sp_local[c].irq_frame=(uintptr_t)f;
  if(c==sp_fault_cpu()&&!sp_local[c].ud)sp_raise_ud();
 }else if(vector==6){
  sp_inside((uintptr_t)f,22*8,cpus[c].tlb_stack,sizeof cpus[c].tlb_stack);sp_inside(f[20],1,cpus[c].irq,sizeof cpus[c].irq);
  if(sp_local[c].nesting!=2||f[17]!=(uintptr_t)sp_ud_site||f[16]!=0)sp_fail("unrecognized-ud");
  sp_local[c].ud++;sp_local[c].ud_frame=(uintptr_t)f;unsigned before=sp_peer_progress(c);sp_local[c].peer_before=before;
  if(topology.count>1){unsigned spin=0;while(sp_peer_progress(c)<before+4){if(++spin==100000000)sp_fail("peer-progress-missing");pause_cpu();}}
  sp_local[c].peer_after=sp_peer_progress(c);sp_raise_gp();f[17]=(uintptr_t)sp_ud_return;
  __atomic_store_n(&sp_fault_done,1,__ATOMIC_RELEASE);
 }else if(vector==13){
  sp_inside((uintptr_t)f,22*8,sp_local[c].emergency,sizeof sp_local[c].emergency);sp_inside(f[20],1,cpus[c].tlb_stack,sizeof cpus[c].tlb_stack);
  if(sp_local[c].nesting!=3||f[17]!=(uintptr_t)sp_gp_site||!f[16])sp_fail("unrecognized-gp");
  if(SP_GUEST_MUTANT!=3)sp_local[c].gp++;sp_local[c].gp_frame=(uintptr_t)f;f[17]=(uintptr_t)sp_gp_return;
 }else sp_fail("unknown-vector");
 sp_local[c].nesting--;
}
void sp_yield(void){unsigned c=cmk2_cpu_index(),t=sp_local[c].current;sp_switch(&sp_local[c].task_sp[t],sp_local[c].idle,sp_local[c].task_fx[t],sp_local[c].idle_fx);}
static __attribute__((noinline)) void sp_nested(unsigned depth,uint64_t seed){
 volatile uint64_t mark[8];for(unsigned i=0;i<8;++i)mark[i]=seed+i+depth;
 if(depth)sp_nested(depth-1,seed+17);else if(sp_register_probe(seed))sp_fail("register-or-SIMD-preservation");
 for(unsigned i=0;i<8;++i)if(mark[i]!=seed+i+depth)sp_fail("nested-frame-corruption");
}
static void sp_task(void){
 unsigned c=cmk2_cpu_index(),t=sp_local[c].current;
 for(unsigned i=0;i<64||!__atomic_load_n(&sp_fault_done,__ATOMIC_ACQUIRE);++i){
  if(i>100000)sp_fail("controlled-fault-not-completed");sp_nested(4,UINT64_C(0x12340000)+c*0x1000+t*0x100+(i&63));__atomic_fetch_add(&sp_local[c].progress,1,__ATOMIC_RELEASE);
 }
 sp_local[c].task_done[t]=1;sp_yield();sp_fail("completed-task-resumed");
}
static void sp_worker(unsigned c){
 uintptr_t actual_stack;__asm__ volatile("mov %%rsp,%0":"=r"(actual_stack));sp_inside(actual_stack,1,cpus[c].boot,sizeof cpus[c].boot);
 sp_local_init(c);
 for(unsigned t=0;t<2;++t){uint64_t *sp=(uint64_t *)(sp_local[c].task_stack[t]+8192);*--sp=0;*--sp=(uintptr_t)sp_task;for(unsigned k=0;k<6;++k)*--sp=0;sp_local[c].task_sp[t]=sp;}
 __atomic_store_n(&sp_local[c].ready,1,__ATOMIC_RELEASE);while(!__atomic_load_n(&sp_go,__ATOMIC_ACQUIRE))pause_cpu();
 /* Check before any interrupt can use the pointer, after BSP startup output
    has finished. The intended refusal has one intact diagnostic publisher. */
 if(cpus[c].tss.ist[0]!=(uintptr_t)(cpus[c].irq+sizeof cpus[c].irq))sp_fail("tss-alias");
 while(!sp_local[c].task_done[0]||!sp_local[c].task_done[1])for(unsigned t=0;t<2;++t)if(!sp_local[c].task_done[t]){sp_local[c].current=t;sp_switch(&sp_local[c].idle,sp_local[c].task_sp[t],sp_local[c].idle_fx,sp_local[c].task_fx[t]);}
 for(unsigned i=0;i<32;++i){if(cpus[c].irq[i]!=0x91||cpus[c].tlb_stack[i]!=0xa2||sp_local[c].emergency[i]!=0xb3)sp_fail("interrupt-canary");for(unsigned t=0;t<2;++t)if(sp_local[c].task_stack[t][i]!=(unsigned char)(0xc0+c*2+t))sp_fail("task-canary");}
 if(sp_local[c].nesting||sp_local[c].irq<128||(c==sp_fault_cpu()&&(sp_local[c].ud!=1||sp_local[c].gp!=1)))sp_fail("fault-accounting");
 __atomic_store_n(&sp_local[c].done,1,__ATOMIC_RELEASE);
}
void sp_bsp(void){
 /* BSP initializes before releasing AP workers; it is its own local-table owner. */
 for(unsigned i=1;i<topology.count;++i){unsigned spin=0;while(!__atomic_load_n(&sp_local[i].ready,__ATOMIC_ACQUIRE)){if(++spin==100000000)sp_fail("worker-not-ready");pause_cpu();}}
 struct sp_span spans[4*SP_PARTS];
 for(unsigned c=0;c<topology.count;++c){
  uintptr_t lows[]={ (uintptr_t)sp_local[c].task_stack[0],(uintptr_t)cpus[c].boot,(uintptr_t)cpus[c].irq,(uintptr_t)sp_local[c].emergency,(uintptr_t)cpus[c].gdt,(uintptr_t)&cpus[c].tss,(uintptr_t)cpus[c].idt,(uintptr_t)sp_local[c].task_stack[1],(uintptr_t)cpus[c].tlb_stack};
  size_t sizes[]={8192,sizeof cpus[c].boot,sizeof cpus[c].irq,sizeof sp_local[c].emergency,sizeof cpus[c].gdt,sizeof cpus[c].tss,sizeof cpus[c].idt,8192,sizeof cpus[c].tlb_stack};
  for(unsigned k=0;k<SP_PARTS;++k)spans[c*SP_PARTS+k]=(struct sp_span){lows[k],lows[k]+sizes[k],c,k};
 }
 if(sp_validate(spans,topology.count*SP_PARTS)!=SP_OK)sp_fail("actual-storage-alias");
 __atomic_store_n(&sp_go,1,__ATOMIC_RELEASE);sp_worker(0);
 for(unsigned c=1;c<topology.count;++c){unsigned spin=0;while(!__atomic_load_n(&sp_local[c].done,__ATOMIC_ACQUIRE)){if(++spin==100000000)sp_fail("worker-not-done");pause_cpu();}}
 for(unsigned c=0;c<topology.count;++c){text("SP_CPU index=");number(c);text(" apic=");number(topology.ids[c]);text(" irq=");number(sp_local[c].irq);text(" ud=");number(sp_local[c].ud);text(" gp=");number(sp_local[c].gp);text(" progress=");number(sp_local[c].progress);text(" peer_before=");number((unsigned)sp_local[c].peer_before);text(" peer_after=");number((unsigned)sp_local[c].peer_after);text(" irq_frame=");ss_u64(sp_local[c].irq_frame);text(" ud_frame=");ss_u64(sp_local[c].ud_frame);text(" gp_frame=");ss_u64(sp_local[c].gp_frame);text("\n");}
 for(unsigned i=0;i<topology.count*SP_PARTS;++i){text("SP_RANGE cpu=");number(spans[i].cpu);text(" kind=");number(spans[i].kind);text(" low=");ss_u64(spans[i].low);text(" high=");ss_u64(spans[i].high);text("\n");}
 text("SP_GUEST_PASS:private-tables-nested-faults-context-state simd=");number(SP_SIMD);text("\n");unsigned v=16;__asm__ volatile("outl %0,$0xf4"::"a"(v));for(;;)__asm__ volatile("cli;hlt");
}
