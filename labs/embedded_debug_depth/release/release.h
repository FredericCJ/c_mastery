#ifndef EMBEDDED_DEBUG_RELEASE_H
#define EMBEDDED_DEBUG_RELEASE_H
#include <stddef.h>
#include <stdint.h>

enum
{
    DT_OK,
    DT_INVALID,
    DT_FULL,
    DT_EMPTY,
    DT_EXHAUSTED,
    DT_RANGE,
    DT_UNIMPLEMENTED
};

struct dt_record
{
    uint32_t sequence, kind, timestamp, payload;
};

struct dt_trace
{
    struct dt_record records[3];
    uint32_t next, dropped;
    unsigned head, count;
};

/* Fresh zero storage, externally serialized. MAX sequence is a retirement
   sentinel. Full consumes an attempted sequence and increments dropped. */
int dt_emit(struct dt_trace *, uint32_t kind, uint32_t timestamp,
            uint32_t payload);
int dt_take(struct dt_trace *, struct dt_record *);

struct dt_critical
{
    void *context;
    uint32_t (*enter)(void *);
    void (*leave)(void *, uint32_t);
};

/* Single-core normal/IRQ callers only. All accesses use this same reliable,
   bounded, nonreentrant masking port. NMI/HardFault/SMP require another proof. */
int dt_irq_emit(struct dt_trace *, struct dt_critical, uint32_t, uint32_t,
                uint32_t);
int dt_irq_take(struct dt_trace *, struct dt_critical, struct dt_record *);

struct dt_interval
{
    uint32_t before, after, frequency;
    unsigned width;
};

struct dt_duration
{
    uint64_t lower_ns, upper_ns;
};

/* Each interval is independently known shorter than one wrap and contains
   no frequency transition. Split at transitions; round each segment outwards. */
int dt_duration(const struct dt_interval *, size_t, struct dt_duration *);

struct dt_image
{
    uint32_t vector, end;
};

int dt_image(uint32_t base, uint32_t bytes, uint32_t vector_offset,
             struct dt_image *);

struct dt_retained
{
    uint32_t version, valid, image, code, check;
};

struct dt_ram
{
    unsigned char data[8], bss[8];
    struct dt_retained crash;
};

/* warm=0 discards all retained state; warm=1 accepts exact v1/current image
   with valid magic and accidental-corruption checksum. No authentication. */
int dt_startup(struct dt_ram *, const unsigned char initial[8], unsigned warm,
               uint32_t image);
int dt_record(struct dt_retained *, uint32_t image, uint32_t code);
#endif
