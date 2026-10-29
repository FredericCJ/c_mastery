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
struct xf_frame {
  uint32_t r[4], r12, stacked_lr, pc, xpsr, fp[16], fpscr;
  uint32_t original_sp;
  unsigned thread, psp, extended, fp_materialized, alignment_word,
      hardware_words;
};
/* Capture policy: do not read a possibly faulted frame or unpublished lazy FP
 * area. */
int xf_frame(const uint32_t *words, size_t available, uint32_t stack_address,
             uint32_t exc_return, unsigned has_fpu, unsigned fp_materialized,
             unsigned stk_align, unsigned stacking_fault, struct xf_frame *out);
struct xf_priority_config {
  unsigned bits, grouping;
};
struct xf_priority {
  uint8_t encoded;
  unsigned logical, group, sub;
};
int xf_priority(struct xf_priority_config, unsigned raw, struct xf_priority *out);
int xf_urgency(struct xf_priority_config, unsigned urgency, unsigned opposite_encoding,
               uint8_t *out);
/* Application API eligibility for the selected threshold convention; not RTOS
 * verification. */
int xf_api_allowed(struct xf_priority_config, unsigned raw, unsigned max_syscall_raw,
                   unsigned *out);
struct xf_irq {
  unsigned exception;
  uint8_t priority;
  unsigned enabled, pending, active, source;
};
/* Active records and current_raw jointly bound execution priority; -1 supplies
 * no additional active priority. Configurable external IRQs only. */
int xf_pick(struct xf_priority_config, const struct xf_irq *, size_t n, int current_raw,
            unsigned primask, unsigned basepri, unsigned *exception);
int xf_pend(struct xf_irq *, unsigned value);
int xf_source(struct xf_irq *, unsigned asserted);
#endif
