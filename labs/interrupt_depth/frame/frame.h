#ifndef CMASTERY_INTERRUPT_FRAME_H
#define CMASTERY_INTERRUPT_FRAME_H
#include <stddef.h>
#include <stdint.h>
enum {
  XF_OK,
  XF_INVALID,
  XF_SHORT,
  XF_UNSUPPORTED,
  XF_FRAME,
  XF_FULL,
  XF_NONE,
  XF_UNIMPLEMENTED = 99
};
typedef struct {
  uint32_t r[4], r12, stacked_lr, pc, xpsr, fp[16], fpscr;
  uint32_t original_sp;
  unsigned thread, psp, extended, fp_materialized, alignment_word,
      hardware_words;
} XfFrame;
/* Capture policy: do not read a possibly faulted frame or unpublished lazy FP
 * area. */
int xf_frame(const uint32_t *words, size_t available, uint32_t stack_address,
             uint32_t exc_return, unsigned has_fpu, unsigned fp_materialized,
             unsigned stk_align, unsigned stacking_fault, XfFrame *out);
typedef struct {
  unsigned bits, grouping;
} XfPriorityConfig;
typedef struct {
  uint8_t encoded;
  unsigned logical, group, sub;
} XfPriority;
int xf_priority(XfPriorityConfig, unsigned raw, XfPriority *out);
int xf_urgency(XfPriorityConfig, unsigned urgency, unsigned opposite_encoding,
               uint8_t *out);
/* Application API eligibility for the selected threshold convention; not RTOS
 * verification. */
int xf_api_allowed(XfPriorityConfig, unsigned raw, unsigned max_syscall_raw,
                   unsigned *out);
typedef struct {
  unsigned exception;
  uint8_t priority;
  unsigned enabled, pending, active, source;
} XfIrq;
/* Active records and current_raw jointly bound execution priority; -1 supplies
 * no additional active priority. Configurable external IRQs only. */
int xf_pick(XfPriorityConfig, const XfIrq *, size_t n, int current_raw,
            unsigned primask, unsigned basepri, unsigned *exception);
int xf_pend(XfIrq *, unsigned value);
int xf_source(XfIrq *, unsigned asserted);
#endif
