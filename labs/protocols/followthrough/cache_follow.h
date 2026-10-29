#ifndef CACHE_FOLLOW_H
#define CACHE_FOLLOW_H
#include "protocol_follow.h"
#include "h745_cache.h"
/* Owned CPU allocation and selected32-bit bus mapping describe the same lines. */
int pf_cached_start(struct pf_driver *, unsigned char **, size_t,
                    uint32_t bus_address, unsigned owned_bytes,
                    fw_cache_emit_fn, void *, struct pf_recovery *);
#endif
