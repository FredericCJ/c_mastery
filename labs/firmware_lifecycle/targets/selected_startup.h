#ifndef B06_SELECTED_STARTUP_H
#define B06_SELECTED_STARTUP_H
#include "boot_storage.h"
/* Cold reset only; no clocks have been reconfigured, caches/MPU off, M4 held.
 * Read callbacks are exact selected registers, writes synchronous MMIO. */
int st_cold_h745(struct fh_io);
int st_cold_f439(struct fh_io);
/* Initialization precedes irreversible peer release. Null/invalid rejects
 * without publication. IO may retain no pointer and is not reentrant. */
int st_release(struct fh_io, const struct bs_selection *);
/* M4 independently authenticates the same pair and consumes only matching
 * generation/sequence/core vector. Returns1 iff mailbox matched unchanged. */
int st_peer_matches(struct fh_io, const struct bs_selection *);
struct fh_io st_target_io(void);
#endif
