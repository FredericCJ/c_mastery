#ifndef P4_CAPTURE_LAYOUT_H
#define P4_CAPTURE_LAYOUT_H
#include <stdint.h>

enum
{
    P4_EVENTS = 64,
    /* Bits of p4_row.flags. */
    P4_ACCEPTED = 1, /* ring took the sample */
    P4_DROPPED = 2, /* ring was full; the sample is lost */
    P4_FRAMED = 4, /* frame encoded and handed to the transmitter */
    P4_DMA_DONE = 8, /* DMA stream quiescent (EN=0, NDTR=0) */
    P4_WIRE_DONE = 16, /* USART TC observed by polling */
    /* This event's send did not finish successfully (begin refused, aborted
       or the next frame not armed); see first_fault. */
    P4_FAILED = 32
};

/*
 * One event. All `*_cycle` fields are DWT cycle-counter values (cycles, not
 * nanoseconds) and are zero when tracing is off.
 *
 * Single IRQ writes acquisition/publication; foreground writes later fields.
 * Export only after EXTI is masked and the selected operation is quiescent,
 * or explicitly identify a contained failed snapshot. No live struct memcpy.
 */
struct p4_row
{
    uint32_t sequence;
    /* Sampled after the pending-bit check and clear, the bounds check and
       raising PA3, so it is a little later than the true handler entry. */
    uint32_t isr_cycle;
    uint32_t publish_cycle; /* after the ring push */
    uint32_t frame_cycle; /* frame encoded */
    uint32_t dma_cycle; /* DMA quiescent */
    uint32_t tc_cycle; /* TC first observed */
    uint32_t flags;
    uint32_t generation; /* transmitter generation that carried the event */
};

/*
 * The experiment's result block. Little-endian, 32-bit fields only, so a
 * host decoder reads it word by word. `state` is 1 running, 2 stopped, 3
 * failed; `first_fault` is nonzero only for state 3 and is never overwritten.
 */
struct p4_capture
{
    uint32_t magic;
    uint32_t version;
    uint32_t trace_enabled;
    uint32_t mode; /* 0 polling, 1 DMA, 2 half duplex */
    uint32_t ring_capacity;
    uint32_t received;
    uint32_t accepted;
    uint32_t dropped;
    uint32_t completed;
    uint32_t state;
    uint32_t clock_start;
    uint32_t clock_end;
    uint32_t first_fault;
    uint32_t fault_generation;
    uint32_t retained; /* 1 if a failed transfer still owns its buffer */
    uint32_t retries;
    struct p4_row rows[P4_EVENTS];
};

enum
{
    P4_CAPTURE_MAGIC = 0x50344331
};

_Static_assert(
    sizeof(struct p4_row) == 32 && sizeof(struct p4_capture) == 2112,
    "Selected capture ABI must match explicit little-endian decoder");
#endif
