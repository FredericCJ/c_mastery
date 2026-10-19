#include <stdint.h>
extern uint32_t cl_stack_top,cl_data_start,cl_data_end,cl_data_load,cl_bss_start,cl_bss_end;
static void semihost(uint32_t op,const void *arg){
  register uint32_t r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;
  __asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");
}
__attribute__((noreturn)) static void rwd_target_stop(unsigned status){
  uint32_t args[2]={0x20026u,status};
  semihost(4,status==0?"RWD_M4_PASS\n":status==1?"RWD_M4_CHECK\n":"RWD_M4_FAULT\n");
  semihost(0x20,args);for(;;){}
}
static int rwd_target_puts(const char *s){semihost(4,s);semihost(4,"\n");return 0;}
#define RWD_TARGET 1
#include "rwlock_tests.c"
#include "rwphase_tests.c"
void cl_reset(void){
  uintptr_t source=(uintptr_t)&cl_data_load;
  for(uintptr_t p=(uintptr_t)&cl_data_start;p<(uintptr_t)&cl_data_end;p+=4,source+=4)*(uint32_t *)p=*(const uint32_t *)source;
  for(uintptr_t p=(uintptr_t)&cl_bss_start;p<(uintptr_t)&cl_bss_end;p+=4)*(uint32_t *)p=0;
  CHECK(rw_policy()==0&&rw_cancelled()==0&&rw_bounds()==0&&rw_starvation()==0);
  CHECK(pf_phases()==0&&pf_upgrade()==0&&pf_cancelled()==0&&pf_boundaries()==0);
  semihost(4,"RWD_M4_LEDGER:eight-cases;actual-CMX-rw+wait-latches;writer-gate;phase-cohorts;upgrade-cancel;retirement\n");
  rwd_target_stop(0);
}
static void fault(void){rwd_target_stop(2);}
__attribute__((section(".vectors"),used)) const void *rwd_vectors[]={&cl_stack_top,cl_reset,fault,fault,fault,fault,fault};
