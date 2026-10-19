#ifndef CMASTERY_ARCHITECTURE_FRAME_LEDGER_H
#define CMASTERY_ARCHITECTURE_FRAME_LEDGER_H
#include "frame.h"
enum { MF_OK, MF_INVALID, MF_SHORT, MF_UNSUPPORTED, MF_STATE, MF_UNIMPLEMENTED=99 };
enum { MF_FLASH, MF_SRAM };
typedef struct { unsigned boot,irq; uint32_t vtor,stack_low,stack_high,code_low,code_high; } MFBoot;
typedef struct { uint32_t reset_fetch[2],reset_backing[2],initial_msp,reset_pc,exception,irq_vector; } MFReset;
typedef struct { uint32_t hardware,callee_preserved,call_clobbered; unsigned r9_platform; } MFOwnership;
typedef struct { uint32_t first_pc,first_sp,second_pc,second_sp; unsigned first_stack,second_stack; } MFReturnOrder;
/* A documentary F439 Rev21 main-flash-bank1/SRAM1 boot-alias selection.
 * All numeric addresses are labels, not dereferenceable host pointers. */
int mf_reset(MFBoot,const uint32_t *table,size_t words,MFReset *out);
/* Basic policy admits only FD/no padding. Extended policy delegates validated
 * architectural variants to the actual imported xf_frame implementation. */
int mf_capture(const uint32_t *,size_t,uint32_t sp,uint32_t er,unsigned extended_policy,
               unsigned fpu,unsigned material,unsigned align,unsigned fault,XfFrame *out);
int mf_ownership(unsigned r9_callee,MFOwnership *out);
int mf_nested(const XfFrame *outer,uint32_t outer_sp,unsigned outer_exception,
              const XfFrame *inner,uint32_t inner_sp,XfPriorityConfig,
              unsigned outer_raw,unsigned inner_raw,MFReturnOrder *out);
#endif
