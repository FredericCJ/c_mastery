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

/*
 * Bytewise form of in_records. The first byte of a record is its length
 * (1..31); the record is committed when its last byte arrives. Returns 0 while
 * a record is incomplete, 1 when one was committed, -1 on a malformed record
 * or when the ring refuses it. After the first -1 the state is terminal
 * (failed) and every later call returns -1. No allocation; the caller owns
 * both objects.
 */
int in_stream(struct in_stream *, uint8_t, algo_ring *);

/*
 * Retires a cancelled operation and discards the queued items. `completed` is
 * a trustworthy observation that the operation stopped, never a guess from
 * elapsed time. Returns 0 when the queue was emptied (each item counted in
 * *discarded), 1 while cancellation is requested but not yet observed
 * complete (the queue is kept), -1 for a null argument or an invalid
 * operation state. *discarded is only incremented, never reset: the caller
 * initialises it.
 */
int in_shutdown(algo_ring *, struct f_async *, unsigned completed,
                uint32_t ticket, unsigned *discarded);

/*
 * Filters `n` samples into the output (capacity `cap` elements), carrying the
 * filter history across calls, and stores the measurement taken between the
 * two samples. Returns 0 on success, -1 on a null argument, n > cap, size
 * overflow or a component failure; on -1 the measurement is not written, but
 * out[] and the filter history may already be partly advanced after a
 * component failure.
 */
int in_filter(struct v4_filter *, const int32_t *, size_t, int32_t *, size_t,
              const struct v4_sample *, const struct v4_sample *,
              struct v4_measurement *);
#ifdef IN_FIRMWARE
/*
 * Starts a DMA transfer and queues its ticket only when the device accepted
 * it. Returns 0, or -1 (nothing queued) if the queue is full, the ticket does
 * not fit in int32_t or the device refuses.
 */
int in_dma_handoff(struct fw_dma *, struct fw_io, uint32_t, algo_ring *);

/*
 * Acquires one ADC sample, filters it into *output and feeds the watchdog.
 * Returns 0 on success, -1 on failure: a null argument, a full queue, an
 * unhealthy watchdog, or a component failure. If acquisition fails, no sample
 * is queued, the watchdog is not fed and *output is unchanged.
 */
int in_gateway(struct fw_io, algo_ring *, struct v4_filter *,
               struct fw_watchdog *, int32_t *);
#endif
#ifdef IN_VM_STORE
/*
 * Compiles and runs a bounded CVM expression, then encodes its integer result
 * as one store page in `page` (capacity at least STORE_PAGE_SIZE). Returns 0,
 * or -1 on a null page, cap < STORE_PAGE_SIZE, or a compile, run, type or
 * encode failure. After a compile or run failure `page` is unchanged.
 */
int in_vm_store(const char *, size_t, uint8_t *, size_t);
#endif
#endif
