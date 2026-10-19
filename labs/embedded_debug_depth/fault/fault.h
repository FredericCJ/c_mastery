#ifndef CMASTERY_DEBUG_FAULT_H
#define CMASTERY_DEBUG_FAULT_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
enum {DF_OK,DF_INVALID,DF_INCOMPLETE,DF_BUSY,DF_AGAIN,DF_IDENTITY,DF_UNSUPPORTED,DF_RANGE,DF_UNKNOWN,DF_UNIMPLEMENTED=99};
enum {DF_STATUS=1,DF_BUS_ADDRESS=2,DF_MEMORY_ADDRESS=4,DF_FRAME=8,DF_IMAGE=16};
typedef struct {
 uint32_t version,valid,cfsr,hfsr,exc_return,bfar,mmfar,frame[8];
 unsigned frame_status;unsigned char image[32];
} DfRecord;
/* Fresh/quiescent init only; first record persists until whole-owner retirement.
 * Source/input/output live disjoint extents, immutable through each call. */
typedef struct {atomic_uint state,secondary;DfRecord record;} DfCapture;
typedef struct {uint32_t cfsr,hfsr,bfar,mmfar,exc_return,stack_address;const uint32_t *words;size_t available;unsigned readable,aligned,stacking_fault;const unsigned char *image;} DfInput;
typedef struct {unsigned precise_bus,address_valid,forced,imprecise;} DfFlags;
typedef struct {uint32_t pc,effective_address,operand;unsigned write,precise;} DfAccess;
void df_init(DfCapture *);
/* Optional hook exists solely for a deterministic nested-capture test.
 * Hook may submit secondary capture, not destroy/reset input or owner. */
int df_capture(DfCapture *,const DfInput *,void (*hook)(void*),void*);
int df_read(const DfCapture *,DfRecord *,unsigned *secondary);
int df_flags(const DfRecord *,DfFlags *);
/* Selected Thumb16 word load/store immediate only, base/value in r0..r3.
 * No address dereference; exact image identity required before decode. */
int df_access(const DfRecord *,const unsigned char image[32],uint32_t code_base,const unsigned char *code,size_t bytes,DfAccess *);
enum {DF_LOCKUP,DF_PENDING,DF_IGNORED};
/* Selected priority<0 cases: precise bus, imprecise bus, MPU data, usage fault. */
int df_refault(unsigned kind,unsigned bfhfnmign,unsigned hfnmiena,unsigned *out);
int df_watermark(const unsigned char *stack,size_t bytes,unsigned char fill,size_t *used);
int df_canary(const unsigned char *bytes,size_t n,unsigned char value,unsigned *intact);
typedef struct {uint32_t software;unsigned exception,extended,padding;} DfLiveFrame;
typedef struct {uint32_t required;unsigned fits;} DfBudget;
int df_budget(const DfLiveFrame *,size_t n,uint32_t allocated,unsigned recursion,unsigned opaque,DfBudget *);
#endif
