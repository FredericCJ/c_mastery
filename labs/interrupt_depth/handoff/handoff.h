#ifndef CMASTERY_INTERRUPT_HANDOFF_H
#define CMASTERY_INTERRUPT_HANDOFF_H
#include "frame.h"
#include <limits.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
enum {
  XI_OK,
  XI_INVALID,
  XI_FULL,
  XI_EMPTY,
  XI_BUSY,
  XI_STATE,
  XI_EXHAUSTED,
  XI_UNSUPPORTED,
  XI_UNIMPLEMENTED = 99
};
enum { XI_RECORD_BYTES = 8, XI_QUEUE_SLOTS = 8, XI_TRUNCATED = 1 };
struct xi_record {
  uint32_t sequence, timestamp, original_length;
  unsigned copied, flags;
  unsigned char bytes[XI_RECORD_BYTES];
};
/* One nonreentrant producer owns p_phase/p_index/dropped; one nonreentrant
 * consumer owns c_active/c_index. Only head/tail are jointly accessed. */
struct xi_queue {
  atomic_uint head, tail;
  unsigned slots, p_phase, p_index, c_active, c_index;
  struct xi_record records[XI_QUEUE_SLOTS];
};
int xi_init(struct xi_queue *, unsigned slots);
int xi_reserve(struct xi_queue *);
int xi_fill(struct xi_queue *, const struct xi_record *);
int xi_commit(struct xi_queue *);
int xi_cancel(struct xi_queue *);
int xi_take(struct xi_queue *, struct xi_record *);
int xi_release(struct xi_queue *);
int xi_push(struct xi_queue *, const struct xi_record *);
int xi_pop(struct xi_queue *, struct xi_record *);
struct xi_service {
  struct xi_queue *queue;
  unsigned active;
  uint32_t acknowledged, dropped, reentrant;
};
int xi_service_init(struct xi_service *, struct xi_queue *);
/* Input has at least min(length,8) live stable bytes. Entry acknowledges this
 * one owned event. A BUSY nested call is not authorized to acknowledge it. */
int xi_capture_begin(struct xi_service *, uint32_t sequence, uint32_t timestamp,
                     const unsigned char *, uint32_t length);
int xi_capture_end(struct xi_service *);
/* Both operations require the SAME external serialization domain. This is
 * deliberately a different ownership contract from the SPSC queue. */
struct xi_latest {
  struct xi_record records[3];
  unsigned first, count;
  uint32_t dropped;
};
void xi_latest_init(struct xi_latest *);
int xi_latest_push(struct xi_latest *, const struct xi_record *);
int xi_latest_pop(struct xi_latest *, struct xi_record *);
/* Serialized mask model. token identity saturates and nested exits are LIFO. */
struct xi_mask {
  struct xf_priority_config config;
  unsigned threshold, value, depth, saved[8];
  uint32_t tokens[8], next;
};
int xi_mask_init(struct xi_mask *, struct xf_priority_config, unsigned threshold,
                 unsigned prior);
int xi_mask_enter(struct xi_mask *, unsigned requested, uint32_t *token);
int xi_mask_leave(struct xi_mask *, uint32_t token);
/* Actual privileged Armv7-M operations, implemented separately in the target
 * port. The caller owns the saved value and restores it exactly once/LIFO. */
uint32_t xi_primask_enter(void);
void xi_primask_leave(uint32_t);
uint32_t xi_basepri_enter(uint32_t);
void xi_basepri_leave(uint32_t);
/* Single-core, privileged, all-accessors-serialized 64-bit logical counter.
 * Neither ISO atomic nor multicore mutual exclusion. No NMI/fault accessor. */
struct xi_wide {
  uint32_t low, high;
};
int xi_wide_add(struct xi_wide *, uint32_t);
struct xi_wide xi_wide_read(const struct xi_wide *);
#endif
