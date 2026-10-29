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

struct in_transfer
{
    uint8_t pending;
    unsigned valid;
};

/* Queue values must be bytes. 0=drained, 1=budget/retry pending, -1=terminal
   adapter failure. The active byte remains owned across failure; after explicit
   recovery invoke again to resume, never replay an already accepted byte. */
int in_drain(algo_ring *, struct in_transfer *, f_writer_fn, void *,
             unsigned budget);

struct in_stream
{
    uint8_t bytes[32];
    size_t length, needed;
    unsigned failed;
};

int in_stream(struct in_stream *, uint8_t, algo_ring *);
int in_shutdown(algo_ring *, struct f_async *, unsigned completed,
                uint32_t ticket, unsigned *discarded);
int in_filter(struct v4_filter *, const int32_t *, size_t, int32_t *, size_t,
              const struct v4_sample *, const struct v4_sample *,
              struct v4_measurement *);
#ifdef IN_FIRMWARE
int in_dma_handoff(struct fw_dma *, struct fw_io, uint32_t, algo_ring *);
int in_gateway(struct fw_io, algo_ring *, struct v4_filter *,
               struct fw_watchdog *, int32_t *);
#endif
#ifdef IN_VM_STORE
int in_vm_store(const char *, size_t, uint8_t *, size_t);
#endif
#endif
