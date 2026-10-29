#ifndef B06_BOOT_PAIR_H
#define B06_BOOT_PAIR_H
#include "flash_maintenance.h"
#include <stddef.h>
struct fb_layout {uint32_t base,limit,stack_low,stack_high;};
struct fb_policy {unsigned char keys[2][32];unsigned present,revoked;uint32_t floor[2],generation_floor;};
struct fb_image {uint32_t version,generation,base,stack,entry,extent;unsigned key,core;unsigned char digest[32];};
/* Distinct live buffers, fixed signature64bytes; output unchanged on rejection.
 * Immutable exact header32+payload514..2048, not a pointer to arbitrary MMIO. */
int fb_admit(const struct fb_policy *,struct fb_layout,unsigned core,const unsigned char *,size_t,
             const unsigned char signature[64],struct fb_image *);
/* No health bit in untrusted metadata. Exact live peer predicate is checked. */
int fb_joint_record(const struct fb_image[2],const struct fw_peer *,uint32_t sequence,unsigned char out[128]);
int fb_record_matches(const unsigned char record[128],const struct fb_image[2]);
/* Privileged thread-mode, quiescent exceptions/DMA, selected owned immutable
 * images. This is a Cortex-M handoff, never a substitute for hardware reset. */
_Noreturn void fb_handoff(uint32_t vector,uint32_t stack,uint32_t entry);
#endif
