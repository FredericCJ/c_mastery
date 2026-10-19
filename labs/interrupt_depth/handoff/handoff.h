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
typedef struct {
  uint32_t sequence, timestamp, original_length;
  unsigned copied, flags;
  unsigned char bytes[XI_RECORD_BYTES];
} XiRecord;
/* One nonreentrant producer owns p_phase/p_index/dropped; one nonreentrant
 * consumer owns c_active/c_index. Only head/tail are jointly accessed. */
typedef struct {
  atomic_uint head, tail;
  unsigned slots, p_phase, p_index, c_active, c_index;
  XiRecord records[XI_QUEUE_SLOTS];
} XiQueue;
int xi_init(XiQueue *, unsigned slots);
int xi_reserve(XiQueue *);
int xi_fill(XiQueue *, const XiRecord *);
int xi_commit(XiQueue *);
int xi_cancel(XiQueue *);
int xi_take(XiQueue *, XiRecord *);
int xi_release(XiQueue *);
int xi_push(XiQueue *, const XiRecord *);
int xi_pop(XiQueue *, XiRecord *);
typedef struct {
  XiQueue *queue;
  unsigned active;
  uint32_t acknowledged, dropped, reentrant;
} XiService;
int xi_service_init(XiService *, XiQueue *);
/* Input has at least min(length,8) live stable bytes. Entry acknowledges this
 * one owned event. A BUSY nested call is not authorized to acknowledge it. */
int xi_capture_begin(XiService *, uint32_t sequence, uint32_t timestamp,
                     const unsigned char *, uint32_t length);
int xi_capture_end(XiService *);
/* Both operations require the SAME external serialization domain. This is
 * deliberately a different ownership contract from the SPSC queue. */
typedef struct {
  XiRecord records[3];
  unsigned first, count;
  uint32_t dropped;
} XiLatest;
void xi_latest_init(XiLatest *);
int xi_latest_push(XiLatest *, const XiRecord *);
int xi_latest_pop(XiLatest *, XiRecord *);
/* Serialized mask model. token identity saturates and nested exits are LIFO. */
typedef struct {
  XfPriorityConfig config;
  unsigned threshold, value, depth, saved[8];
  uint32_t tokens[8], next;
} XiMask;
int xi_mask_init(XiMask *, XfPriorityConfig, unsigned threshold,
                 unsigned prior);
int xi_mask_enter(XiMask *, unsigned requested, uint32_t *token);
int xi_mask_leave(XiMask *, uint32_t token);
/* Actual privileged Armv7-M operations, implemented separately in the target
 * port. The caller owns the saved value and restores it exactly once/LIFO. */
uint32_t xi_primask_enter(void);
void xi_primask_leave(uint32_t);
uint32_t xi_basepri_enter(uint32_t);
void xi_basepri_leave(uint32_t);
/* Single-core, privileged, all-accessors-serialized 64-bit logical counter.
 * Neither ISO atomic nor multicore mutual exclusion. No NMI/fault accessor. */
typedef struct {
  uint32_t low, high;
} XiWide;
int xi_wide_add(XiWide *, uint32_t);
XiWide xi_wide_read(const XiWide *);
#endif
