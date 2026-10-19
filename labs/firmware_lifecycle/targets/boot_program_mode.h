#ifndef B06_BOOT_PROGRAM_MODE_H
#define B06_BOOT_PROGRAM_MODE_H
/* Privileged trusted bank2 service only. Peers and DMA quiescent; clocks,
 * caches and controller owned. Leaves application RO MPU, not IRQ masking.
 * Caller supplies no untrusted code/data until reset. */
void fb_program_mode(void);
#endif
