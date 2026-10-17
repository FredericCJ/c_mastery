#include "fiber.h"
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CR_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {unsigned calls,fail;uintptr_t local_address;int rounding;} Probe;
static void work(COFiber *f,void *ctx){
 Probe *p=ctx;volatile unsigned local=17; /* actual fiber stack object */
 /* ASan may relocate local into its fake stack; inspect architectural RSP
  * for the actual stack-switch claim, retain local for lifetime checks. */
 __asm__ volatile("movq %%rsp,%0":"=r"(p->local_address));
 if(fegetround()!=FE_TONEAREST)p->fail++;
 if(fesetround(FE_DOWNWARD))p->fail++;
 p->rounding=fegetround();p->calls++;
 if(co_fiber_yield(f))p->fail++;
 if(local!=17||fegetround()!=FE_DOWNWARD)p->fail++;
 local=29;p->calls++;
 if(co_fiber_yield(f))p->fail++;
 if(local!=29||fegetround()!=FE_DOWNWARD)p->fail++;
 p->calls++;
}
int main(void){
 const size_t size=131072;void *stack=aligned_alloc(16,size);CHECK(stack);
 COFiber f;Probe p={0};CHECK(fesetround(FE_TONEAREST)==0);
 CHECK(co_fiber_init(&f,stack,size,work,&p)==0);
 CHECK(co_fiber_resume(&f)==4&&p.calls==1&&p.fail==0);
 CHECK(p.local_address>=(uintptr_t)stack&&p.local_address<(uintptr_t)stack+size);
 CHECK(fegetround()==FE_TONEAREST&&p.rounding==FE_DOWNWARD);
 CHECK(co_fiber_destroy(&f)==3); /* suspended stack is still owned */
 CHECK(fesetround(FE_UPWARD)==0);
 CHECK(co_fiber_resume(&f)==4&&p.calls==2&&p.fail==0);
 CHECK(fegetround()==FE_UPWARD);
 CHECK(co_fiber_resume(&f)==7&&p.calls==3&&p.fail==0);
 CHECK(co_fiber_resume(&f)==3);
 CHECK(co_fiber_destroy(&f)==0);CHECK(co_fiber_destroy(&f)==3);
 CHECK(fesetround(FE_TONEAREST)==0);free(stack);
 puts("fiber: real stack, resume lifetime, FP control and destruction checks passed");return 0;
}
