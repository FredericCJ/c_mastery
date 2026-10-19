#define _DEFAULT_SOURCE
#include "fiber_follow.h"
#include "coroutines.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#ifndef FF_MUTANT
#define FF_MUTANT 0
#endif
static void enter(COFiber*f,void*v){(void)f;CFFiber*x=v;x->run(x,x->argument);}
int cf_fiber_create(CFFiber*x,size_t requested,CFFiberFn run,void*ctx){
 if(!x||!run)return CO_INVALID;long raw=sysconf(_SC_PAGESIZE);if(raw<=0)return CO_IO;size_t page=(size_t)raw;
 if(requested<65536||requested>16*1024*1024||requested>SIZE_MAX-page+1)return CO_RANGE;
 size_t size=(requested+page-1)/page*page;if(page>SIZE_MAX/2||size>SIZE_MAX-2*page)return CO_RANGE;
 *x=(CFFiber){0};
 size_t total=size+2*page;void*base=mmap(NULL,total,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);if(base==MAP_FAILED)return CO_RANGE;
 /* Publish cleanup ownership before another fallible operation. State 4 is
  * a dead/uninitialized backend: destroy must only release the mapping. */
 *x=(CFFiber){.fiber={.state=4},.mapping=base,.mapping_size=total,.page_size=page,.run=run,.argument=ctx};
 unsigned char*stack=(unsigned char*)base+page;
 if(mprotect(stack,size,PROT_READ|PROT_WRITE)){if(!munmap(base,total))x->mapping=NULL;return CO_IO;}
 /* x is the final stable address before the underlying fiber records it. */
 int r=co_fiber_init(&x->fiber,stack,size,enter,x);
 if(r){if(!munmap(base,total))x->mapping=NULL;return r;}return CO_OK;
}
int cf_fiber_resume(CFFiber*x){if(!x||!x->mapping)return CO_INVALID;return co_fiber_resume(&x->fiber);}
int cf_fiber_suspend(CFFiber*x){if(!x||!x->mapping)return CO_INVALID;if(x->cancel)return CO_IO;int r=co_fiber_yield(&x->fiber);return r?r:x->cancel?CO_IO:CO_OK;}
int cf_fiber_cancel(CFFiber*x){if(!x||!x->mapping)return CO_INVALID;if(x->fiber.state>=3)return CO_STATE;if(FF_MUTANT!=2)x->cancel=1;return CO_OK;}
int cf_fiber_destroy(CFFiber*x){
 if(!x||!x->mapping)return CO_INVALID;
 /* A failed unmap retains the mapping obligation, while backend destruction
  * has already happened. Retry only the unmap, not detector destruction. */
 if(FF_MUTANT!=1&&x->fiber.state!=4){int r=co_fiber_destroy(&x->fiber);if(r)return r;}
 if(munmap(x->mapping,x->mapping_size))return CO_IO;x->mapping=NULL;return CO_OK;
}
