#include "cache_follow.h"
#include <stdio.h>
#include <stdlib.h>
#define C(x) do{if(!(x)){fprintf(stderr,"PC_CHECK:%s:%d\n",#x,__LINE__);exit(1);}}while(0)
typedef struct {unsigned events,busy,frees;enum FWCacheOp last;void *owner;} Env;
static void emit(void *p,enum FWCacheOp op,uint32_t a){Env *e=p;if(op==FW_CLEAN)C(a==0x30040020u||a==0x30040040u);e->last=op;++e->events;}
static uint32_t read_reg(void *p,unsigned r){return r==FW_DMA?((Env*)p)->busy:0;}
static void write_reg(void *p,unsigned r,uint32_t v){Env *e=p;if(r==FW_DMA&&v){C(e->events>=4&&e->last==FW_ISB);e->busy=1;}}
static void release(void *p,void *v){Env *e=p;C(e->owner==v);free(v);e->owner=0;++e->frees;}
int main(void)
{
 Env e={0};PFDriver d;FwIo io={&e,read_reg,write_reg};C(pf_driver_init(&d,io,1,release,&e)==PT_OK);unsigned char *p=aligned_alloc(32,64);C(p);e.owner=p;PFRecovery r;
 unsigned char *bad=p+1;C(pf_cached_start(&d,&bad,1,0x30040020u,32,emit,&e,&r)==PT_RANGE&&bad==p+1&&!e.events);
 C(pf_cached_start(&d,&p,40,0x30040020u,32,emit,&e,&r)==PT_INVALID&&!e.events);
 C(pf_cached_start(&d,&p,40,0x30040020u,64,emit,&e,&r)==PT_OK&&!p&&e.events==5&&d.owned==e.owner);
 C(pf_driver_complete(&r,1,0)==PT_PENDING&&!e.frees);e.busy=0;C(pf_driver_complete(&r,1,1)==PT_OK&&e.frees==1);puts("PC_PASS:exclusive clean/complete before actual DMA publication");
}
