#include "cache_follow.h"
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif
int pf_cached_start(struct pf_driver *d,unsigned char **owner,size_t n,uint32_t bus,unsigned extent,fw_cache_emit_fn emit,void *ctx,struct pf_recovery *out)
{
 if(!d||!owner||!*owner||!out||!emit||!n||n>extent||(extent!=32&&extent!=64))return PT_INVALID;
 if(PC_MUTANT!=2&&(((uintptr_t)*owner&31u)||(bus&31u)))return PT_RANGE;
 if(bus>UINT32_MAX-(extent-1u))return PT_RANGE;
 if(d->owned||d->reset_pending||d->protocol.device.state!=PT_READY)return PT_STATE;
 if(d->dma.generation==UINT32_MAX||d->protocol.device.generation==UINT32_MAX)return PT_RANGE;
 if(PC_MUTANT!=1&&!fw_cache_range(bus,extent,FW_CLEAN,emit,ctx))return PT_RANGE;
 return pf_driver_start(d,owner,n,0,out);
}
