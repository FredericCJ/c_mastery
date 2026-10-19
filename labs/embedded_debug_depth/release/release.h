#ifndef EMBEDDED_DEBUG_RELEASE_H
#define EMBEDDED_DEBUG_RELEASE_H
#include <stddef.h>
#include <stdint.h>
enum { DT_OK,DT_INVALID,DT_FULL,DT_EMPTY,DT_EXHAUSTED,DT_RANGE,DT_UNIMPLEMENTED };
typedef struct {uint32_t sequence,kind,timestamp,payload;} DtRecord;
typedef struct {DtRecord records[3];uint32_t next,dropped;unsigned head,count;} DtTrace;
/* Fresh zero storage, externally serialized. MAX sequence is a retirement
   sentinel. Full consumes an attempted sequence and increments dropped. */
int dt_emit(DtTrace *,uint32_t kind,uint32_t timestamp,uint32_t payload);
int dt_take(DtTrace *,DtRecord *);
typedef struct {void *context;uint32_t (*enter)(void *);void (*leave)(void *,uint32_t);} DtCritical;
/* Single-core normal/IRQ callers only. All accesses use this same reliable,
   bounded, nonreentrant masking port. NMI/HardFault/SMP require another proof. */
int dt_irq_emit(DtTrace *,DtCritical,uint32_t,uint32_t,uint32_t);
int dt_irq_take(DtTrace *,DtCritical,DtRecord *);
typedef struct {uint32_t before,after,frequency;unsigned width;} DtInterval;
typedef struct {uint64_t lower_ns,upper_ns;} DtDuration;
/* Each interval is independently known shorter than one wrap and contains
   no frequency transition. Split at transitions; round each segment outwards. */
int dt_duration(const DtInterval *,size_t,DtDuration *);
typedef struct {uint32_t vector,end;} DtImage;
int dt_image(uint32_t base,uint32_t bytes,uint32_t vector_offset,DtImage *);
typedef struct {uint32_t version,valid,image,code,check;} DtRetained;
typedef struct {unsigned char data[8],bss[8];DtRetained crash;} DtRam;
/* warm=0 discards all retained state; warm=1 accepts exact v1/current image
   with valid magic and accidental-corruption checksum. No authentication. */
int dt_startup(DtRam *,const unsigned char initial[8],unsigned warm,uint32_t image);
int dt_record(DtRetained *,uint32_t image,uint32_t code);
#endif
