#ifndef CACHE_FOLLOW_H
#define CACHE_FOLLOW_H
#include "protocol_follow.h"
#include "h745_cache.h"
/* Owned CPU allocation and selected32-bit bus mapping describe the same lines. */
int pf_cached_start(PFDriver *,unsigned char **,size_t,uint32_t bus_address,
                    unsigned owned_bytes,FWCacheEmit,void *,PFRecovery *);
#endif
