#ifndef EMBEDDED_DEBUG_PERIPHERAL_H
#define EMBEDDED_DEBUG_PERIPHERAL_H
#include <stdint.h>
enum { DP_OK, DP_INVALID, DP_RANGE, DP_WAIT, DP_UNIMPLEMENTED };
enum { DP_GATE=1, DP_MODE=2, DP_SEQUENCE=4, DP_ROUTE=8, DP_TRIGGER=16 };
enum { DP_READBACK=1, DP_ACCEPTED=2, DP_EDGE=4, DP_CAPTURED=8 };
enum { DP_PREDICTED, DP_MODEL, DP_MEASURED };
/* Original Boolean diagnostic plant. A successful register readback is not
   an observed external edge. All live input/output objects are disjoint. */
int dp_predict(unsigned configuration, unsigned *observations);
struct dp_calibration {
  uint64_t reference_ns, anchor_tick, ns_per_tick, uncertainty_ns;
  unsigned provenance;
};
struct dp_correlation { uint64_t earliest_ns, latest_ns; unsigned compatible, provenance; };
/* A supplied common-clock calibration is a premise, not inferred from a log.
   Compatibility of closed time intervals does not prove causation. */
int dp_correlate(const struct dp_calibration *, uint64_t capture_tick,
                 uint64_t logical_begin_ns, uint64_t logical_end_ns,
                 struct dp_correlation *);
enum { DP_MASTER_DUPLEX, DP_SLAVE_RECEIVE, DP_SLAVE_TRANSMIT };
/* Selected errata-aware end predicate. `last_received` means the last RX
   item was consumed. Slave TX requires a justified remaining-frame bound
   measured from the final TXE event. It is not an arbitrary timeout. */
int dp_spi_end(unsigned mode, unsigned txe, unsigned busy,
               unsigned last_received, unsigned final_txe_seen,
               uint32_t elapsed, uint32_t frame_bound);
struct dp_progress {
  unsigned dma_complete_at, reset_at, observed_at, fed, remaining;
};
/* Discrete original schedule: DMA beat, watchdog decrement/reset, then an
   unhalted CPU observes completion and feeds. No real time/peripheral claim. */
int dp_progress(unsigned steps, uint64_t halted, unsigned watchdog_frozen,
                unsigned watchdog_budget, unsigned dma_bytes, struct dp_progress *);
#endif
