#include "pi.h"
extern uint32_t cl_stack_top,cl_data_start,cl_data_end,cl_data_load,cl_bss_start,cl_bss_end;
static cmx_kernel kernel;
static cmx_mutex a,b;
static void semihost(uint32_t op,const void *arg){register uint32_t r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
__attribute__((noreturn)) static void stop(unsigned status){uint32_t arguments[2]={0x20026u,status};semihost(4,status==0?"CDPI_M4_PASS\n":status==1?"CDPI_M4_CHECK\n":"CDPI_M4_FAULT\n");semihost(0x20,arguments);for(;;){}}
#define CHECK(x) do{if(!(x))stop(1);}while(0)
static void exercise(void){
  cmx_init(&kernel,CMX_FIXED);
  CHECK(cmx_task_add(&kernel,9,0,0x20006000,256)==0);
  CHECK(cmx_task_add(&kernel,7,0,0x20006200,256)==1);
  CHECK(cmx_task_add(&kernel,3,0,0x20006400,256)==2);
  CHECK(cmx_mutex_init(&kernel,&a,0,0)==CMX_OK && cmx_mutex_init(&kernel,&b,0,0)==CMX_OK);
  CHECK(cmx_mutex_lock(&kernel,&a,0,cmx_forever())==CMX_OK);
  CHECK(cmx_mutex_lock(&kernel,&b,1,cmx_forever())==CMX_OK);
  CHECK(cmx_mutex_lock(&kernel,&a,1,cmx_forever())==CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&kernel,&b,2,cmx_forever())==CMX_BLOCKED);
  struct cdpi_stats stats;
  CHECK(cdpi_reprioritize(&kernel,0,11,&stats)==CMX_OK);
  CHECK(kernel.task[0].priority==3 && kernel.task[1].priority==3 && kernel.task[0].base_priority==11 && stats.donated_pairs==3);
  CHECK(cdpi_reprioritize(&kernel,2,6,&stats)==CMX_OK && kernel.task[0].priority==6 && kernel.task[1].priority==6);
  CHECK(cmx_schedule(&kernel)==0);
  CHECK(cmx_cancel(&kernel,2)==CMX_OK && kernel.task[0].priority==7);
  CHECK(cmx_cancel(&kernel,1)==CMX_OK && kernel.task[0].priority==11);
  CHECK(cmx_invariant(&kernel));
}
void cl_reset(void){uintptr_t source=(uintptr_t)&cl_data_load;for(uintptr_t p=(uintptr_t)&cl_data_start;p<(uintptr_t)&cl_data_end;p+=4,source+=4)*(uint32_t *)p=*(const uint32_t *)source;for(uintptr_t p=(uintptr_t)&cl_bss_start;p<(uintptr_t)&cl_bss_end;p+=4)*(uint32_t *)p=0;exercise();stop(0);}
static void fault(void){stop(2);}
__attribute__((section(".vectors"),used)) const void *cdpi_vectors[]={&cl_stack_top,cl_reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};
