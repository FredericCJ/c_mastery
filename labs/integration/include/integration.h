#ifndef CMASTERY_INTEGRATION_H
#define CMASTERY_INTEGRATION_H
#include "algo.h"
#include "foundation.h"
#include "v4_algo.h"
#include "v4_foundation.h"
#ifdef IN_FIRMWARE
#include "firmware_io.h"
#endif
/* Wire record: nonzero one-byte length followed by that many signed decimal
   ASCII bytes. Commit complete records in order. A malformed record publishes
   nothing; earlier complete records remain committed. */
int in_records(const uint8_t *, size_t, algo_ring *, size_t *consumed);
typedef struct {
  uint8_t pending;
  unsigned valid;
} InTransfer;
/* Queue values must be bytes. 0=drained, 1=budget/retry pending, -1=terminal
   adapter failure. The active byte remains owned across failure; after explicit
   recovery invoke again to resume, never replay an already accepted byte. */
int in_drain(algo_ring *, InTransfer *, FWriter, void *, unsigned budget);
typedef struct {
  uint8_t bytes[32];
  size_t length, needed;
  unsigned failed;
} InStream;
int in_stream(InStream *, uint8_t, algo_ring *);
int in_shutdown(algo_ring *, FAsync *, unsigned completed, uint32_t ticket,
                unsigned *discarded);
int in_filter(V4Filter *, const int32_t *, size_t, int32_t *, size_t,
              const V4Sample *, const V4Sample *, V4Measurement *);
#ifdef IN_FIRMWARE
int in_dma_handoff(FwDma *, FwIo, uint32_t, algo_ring *);
int in_gateway(FwIo, algo_ring *, V4Filter *, FwWatchdog *, int32_t *);
#endif
#ifdef IN_VM_STORE
int in_vm_store(const char *, size_t, uint8_t *, size_t);
#endif
#endif
