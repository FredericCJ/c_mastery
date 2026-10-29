#ifndef B06_H745_CACHE_H
#define B06_H745_CACHE_H
#include <stdint.h>
enum fw_cache_op { FW_DSB, FW_ISB, FW_CLEAN, H7_INVALIDATE };
typedef void (*fw_cache_emit_fn)(void *, enum fw_cache_op, uint32_t);
/* The caller exclusively owns whole lines; invalidate never protects dirty neighbours. */
int fw_cache_range(uint32_t address, uint32_t length, enum fw_cache_op,
                   fw_cache_emit_fn, void *);
enum fw_peer_op { FW_BEGIN, FW_END, FW_HEALTHY, FW_FAIL_PEER, FW_CONFIRM };
enum fw_peer_result { H7_OK, H7_INVALID, H7_STALE, H7_BUSY, H7_FAILED };
struct fw_peer {
    uint32_t generation, health, failed, outstanding[2], confirmed, reserved[2];
};
_Static_assert(sizeof(struct fw_peer) == 32, "One exclusively owned cache line");
enum fw_peer_result fw_peer_step(struct fw_peer *, unsigned core, uint32_t generation,
                               enum fw_peer_op);
/* Privileged, selected H745 build only; no clock/MPU/reset configuration occurs. */
int fw_h745_cache(uint32_t address, uint32_t length, enum fw_cache_op);
enum fw_peer_result fw_h745_step(uint32_t generation, enum fw_peer_op);
#endif
