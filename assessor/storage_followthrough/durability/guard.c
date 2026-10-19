#include "guard.h"
#ifndef DG_MUTANT
#define DG_MUTANT 0
#endif
#ifndef DG_MAGIC
#define DG_MAGIC UINT32_C(0x43535432)
#endif
static uint32_t u32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t u64(const uint8_t *p){return (uint64_t)u32(p)|(uint64_t)u32(p+4)<<32;}
int dg_validate(store *source,const uint8_t commit[4096]){
 if(DG_MUTANT==1)return STORE_OK;
 store candidate={0};candidate.io=source->io;candidate.generation=u64(commit+8);candidate.checkpoint_generation=source->checkpoint_generation;candidate.wal_active=1;
 uint8_t metadata[4096];int r=store_page_read(&candidate,0,metadata);if(r)return r;
 if(u32(metadata)!=DG_MAGIC||u32(metadata+4)!=10||u64(metadata+8)!=candidate.generation||u32(metadata+16)!=0)return STORE_CORRUPT;
 candidate.root=u32(metadata+20);candidate.used=u32(metadata+24);
 if(!candidate.root||candidate.root>=32||!(candidate.used&1u)||!(candidate.used&(UINT32_C(1)<<candidate.root)))return STORE_CORRUPT;
 return store_check(&candidate);
}
