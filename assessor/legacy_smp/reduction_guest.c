/* Original integer reduction adapter appended to completed startup. */
#include "reduction.h"
#include "reduction.c"
#ifndef SRG_MUTANT
#define SRG_MUTANT 0
#endif
static SrWorld srg_world;static uint64_t srg_values[257];static uint8_t srg_bytes[257];static atomic_uint srg_visits[257];
static unsigned srg_ready[4],srg_command,srg_done[4],srg_observed[4],srg_errors[4],srg_failure;
static uint64_t srg_cycles[4];
static uint64_t srg_ticks(void){unsigned lo,hi;__asm__ volatile("lfence;rdtsc":"=a"(lo),"=d"(hi)::"memory");return (uint64_t)hi<<32|lo;}
static void srg_fail(const char *why){text("SR_GUEST_CHECK:");text(why);text("\n");unsigned code=17;__asm__ volatile("outl %0,$0xf4"::"a"(code));for(;;)__asm__ volatile("cli;hlt");}
static void srg_job(unsigned worker){uint64_t start=srg_ticks();srg_observed[worker]=cmk2_cpu_index();srg_errors[worker]=(unsigned)sr_work(&srg_world,worker,srg_failure&&worker+1==topology.count?0:SIZE_MAX);srg_cycles[worker]=srg_ticks()-start;}
static void srg_worker(unsigned cpu){
 __atomic_store_n(&srg_ready[cpu],1,__ATOMIC_RELEASE);unsigned prior=0;
 for(;;){unsigned command=__atomic_load_n(&srg_command,__ATOMIC_ACQUIRE);if(command==prior){pause_cpu();continue;}prior=command;if(command==99){__atomic_store_n(&srg_done[cpu],command,__ATOMIC_RELEASE);for(;;)__asm__ volatile("cli;hlt");}if(!SRG_MUTANT)srg_job(cpu);__atomic_store_n(&srg_done[cpu],command,__ATOMIC_RELEASE);}
}
static void srg_case(unsigned sequence,size_t count,unsigned mode,unsigned failure){
 if(sr_init(&srg_world,mode==SR_BYTES?(const void *)srg_bytes:(const void *)srg_values,srg_visits,count,topology.count,mode)!=SR_OK)srg_fail("init");
 srg_failure=failure;SrResult out={0};out.unsigned_sum=91;if(sr_finish(&srg_world,&out)!=SR_INCOMPLETE||out.unsigned_sum!=91)srg_fail("early-completion-missing-owned-indices");
 uint64_t start=srg_ticks();__atomic_store_n(&srg_command,sequence,__ATOMIC_RELEASE);srg_job(0);if(SRG_MUTANT)for(unsigned i=1;i<topology.count;i++)srg_job(i);
 for(unsigned cpu=1;cpu<topology.count;cpu++)for(unsigned n=0;__atomic_load_n(&srg_done[cpu],__ATOMIC_ACQUIRE)!=sequence;n++){if(n==100000000)srg_fail("worker-progress");pause_cpu();}
 unsigned result=(unsigned)sr_finish(&srg_world,&out);uint64_t elapsed=srg_ticks()-start;
 if(failure){if(result!=SR_FAILED||out.unsigned_sum!=91)srg_fail("failed-worker-published-total");}
 else{if(result!=SR_OK||out.count!=count)srg_fail("reduction-result");uint64_t expected=0;for(size_t i=0;i<count;i++)expected+=srg_values[i];if(mode==SR_UNSIGNED&&out.unsigned_sum!=expected)srg_fail("integer-oracle");for(size_t i=0;i<count;i++)if(atomic_load(srg_visits+i)!=1)srg_fail("index-ownership");}
 text("SR_CASE sequence=");number(sequence);text(" count=");number((unsigned)count);text(" mode=");number(mode);text(" failed=");number(failure);text(" total=");ss_u64(mode==SR_BYTES?out.hash:out.unsigned_sum);text(" sender_ticks=");ss_u64(elapsed);text("\n");
 for(unsigned cpu=0;cpu<topology.count;cpu++){
  if(srg_observed[cpu]!=cpu)srg_fail("all-workers-one-CPU");SrRange r;if(sr_partition(count,topology.count,cpu,&r)!=SR_OK)srg_fail("range");
  if(srg_errors[cpu]!=(failure&&cpu+1==topology.count?SR_FAILED:SR_OK))srg_fail("worker-error");
  text("SR_PART sequence=");number(sequence);text(" worker=");number(cpu);text(" cpu=");number(srg_observed[cpu]);text(" apic=");number(topology.ids[cpu]);text(" begin=");number((unsigned)r.begin);text(" end=");number((unsigned)r.end);text(" completed=");number((unsigned)srg_world.slot[cpu].value.count);text(" status=");number(srg_errors[cpu]);text(" active_ticks=");ss_u64(srg_cycles[cpu]);text("\n");
 }
 if(mode==SR_BYTES&&!failure){text("SR_HIST");for(unsigned b=0;b<16;b++){text(" b");number(b);text("=");number((unsigned)out.bins[b]);}text("\n");}
}
void srg_bsp(void){
 for(unsigned cpu=1;cpu<topology.count;cpu++)while(!__atomic_load_n(&srg_ready[cpu],__ATOMIC_ACQUIRE))pause_cpu();
 for(unsigned i=0;i<257;i++){srg_values[i]=(uint64_t)(i+1)*(i+1);srg_bytes[i]=(uint8_t)((i*7+3)%16);}
 srg_case(1,0,SR_UNSIGNED,0);srg_case(2,1,SR_UNSIGNED,0);srg_case(3,10,SR_UNSIGNED,0);srg_case(4,257,SR_UNSIGNED,0);srg_case(5,257,SR_UNSIGNED,1);srg_case(6,257,SR_BYTES,0);
 __atomic_store_n(&srg_command,99,__ATOMIC_RELEASE);for(unsigned c=1;c<topology.count;c++)while(__atomic_load_n(&srg_done[c],__ATOMIC_ACQUIRE)!=99)pause_cpu();
 text("SR_GUEST_PASS:actual-static-owned-reduction cpus=");number(topology.count);text(" cases=6\n");unsigned code=16;__asm__ volatile("outl %0,$0xf4"::"a"(code));for(;;)__asm__ volatile("cli;hlt");
}
