#ifndef P4_CAPTURE_LAYOUT_H
#define P4_CAPTURE_LAYOUT_H
#include <stdint.h>
enum {P4_EVENTS=64,P4_ACCEPTED=1,P4_DROPPED=2,P4_FRAMED=4,P4_DMA_DONE=8,P4_WIRE_DONE=16,P4_FAILED=32};
/* Single IRQ writes acquisition/publication; foreground writes later fields.
   Export only after EXTI is masked and the selected operation is quiescent,
   or explicitly identify a contained failed snapshot. No live struct memcpy. */
typedef struct {uint32_t sequence,isr_cycle,publish_cycle,frame_cycle,dma_cycle,tc_cycle,flags,generation;} P4Row;
typedef struct {
    uint32_t magic,version,trace_enabled,mode,ring_capacity;
    uint32_t received,accepted,dropped,completed,state,clock_start,clock_end;
    uint32_t first_fault,fault_generation,retained,retries;
    P4Row rows[P4_EVENTS];
} P4Capture;
enum {P4_CAPTURE_MAGIC=0x50344331};
_Static_assert(sizeof(P4Row)==32 && sizeof(P4Capture)==2112,"Selected capture ABI must match explicit little-endian decoder");
#endif
