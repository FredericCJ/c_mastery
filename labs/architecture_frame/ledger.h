#ifndef CMASTERY_ARCHITECTURE_FRAME_LEDGER_H
#define CMASTERY_ARCHITECTURE_FRAME_LEDGER_H
#include "frame.h"
enum { MF_OK, MF_INVALID, MF_SHORT, MF_UNSUPPORTED, MF_STATE, MF_UNIMPLEMENTED=99 };
enum { MF_FLASH, MF_SRAM };
struct mf_boot { unsigned boot,irq; uint32_t vtor,stack_low,stack_high,code_low,code_high; };
struct mf_reset { uint32_t reset_fetch[2],reset_backing[2],initial_msp,reset_pc,exception,irq_vector; };
struct mf_ownership { uint32_t hardware,callee_preserved,call_clobbered; unsigned r9_platform; };
struct mf_return_order { uint32_t first_pc,first_sp,second_pc,second_sp; unsigned first_stack,second_stack; };
/* A documentary F439 Rev21 main-flash-bank1/SRAM1 boot-alias selection.
 * All numeric addresses are labels, not dereferenceable host pointers. */
int mf_reset(struct mf_boot,const uint32_t *table,size_t words,struct mf_reset *out);
/* Basic policy admits only FD/no padding. Extended policy delegates validated
 * architectural variants to the actual imported xf_frame implementation. */
int mf_capture(const uint32_t *,size_t,uint32_t sp,uint32_t er,unsigned extended_policy,
               unsigned fpu,unsigned material,unsigned align,unsigned fault,struct xf_frame *out);
int mf_ownership(unsigned r9_callee,struct mf_ownership *out);
int mf_nested(const struct xf_frame *outer,uint32_t outer_sp,unsigned outer_exception,
              const struct xf_frame *inner,uint32_t inner_sp,struct xf_priority_config,
              unsigned outer_raw,unsigned inner_raw,struct mf_return_order *out);
#endif
