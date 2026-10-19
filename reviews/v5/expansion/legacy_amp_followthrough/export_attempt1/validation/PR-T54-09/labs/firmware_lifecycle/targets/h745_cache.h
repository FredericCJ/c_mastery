#ifndef B06_H745_CACHE_H
#define B06_H745_CACHE_H
#include <stdint.h>
enum FWCacheOp { FW_DSB, FW_ISB, FW_CLEAN, H7_INVALIDATE };
typedef void (*FWCacheEmit)(void *, enum FWCacheOp, uint32_t);
/* The caller exclusively owns whole lines; invalidate never protects dirty neighbours. */
int fw_cache_range(uint32_t address, uint32_t length, enum FWCacheOp,
                   FWCacheEmit, void *);
enum FWPeerOp { FW_BEGIN, FW_END, FW_HEALTHY, FW_FAIL_PEER, FW_CONFIRM };
enum FWPeerResult { H7_OK, H7_INVALID, H7_STALE, H7_BUSY, H7_FAILED };
typedef struct {
    uint32_t generation, health, failed, outstanding[2], confirmed, reserved[2];
} FWPeer;
_Static_assert(sizeof(FWPeer) == 32, "One exclusively owned cache line");
enum FWPeerResult fw_peer_step(FWPeer *, unsigned core, uint32_t generation,
                               enum FWPeerOp);
/* Privileged, selected H745 build only; no clock/MPU/reset configuration occurs. */
int fw_h745_cache(uint32_t address, uint32_t length, enum FWCacheOp);
enum FWPeerResult fw_h745_step(uint32_t generation, enum FWPeerOp);
#endif
