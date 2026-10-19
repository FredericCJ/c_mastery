#ifndef CMASTERY_REALTIME_CEILING_H
#define CMASTERY_REALTIME_CEILING_H
#include "cmx.h"
#define SC_TASKS 8u
#define SC_RESOURCES 4u
enum sc_mode { SC_INHERITANCE, SC_IMMEDIATE };
enum sc_context { SC_TASK_CONTEXT, SC_INTERRUPT_CONTEXT };
typedef struct {
  unsigned tasks, resources, priority[SC_TASKS], users[SC_RESOURCES],
      ceiling[SC_RESOURCES], strict_nesting;
} SCConfig;
typedef struct {
  cmx_kernel kernel;
  cmx_mutex mutex[SC_RESOURCES];
  SCConfig config;
  unsigned held[SC_TASKS], pending[SC_TASKS];
  enum sc_mode mode;
} SCWorld;
/* Stable externally serialized model storage; caller never mutates fields.
 * Initialize fresh storage only; no retained pointer survives destruction.
 * Init is transactional. Configuration and task base priorities are frozen.
 * Resource rank is its index. Acquisitions are strictly increasing; optional
 * strict_nesting also requires reverse release order. Only the latter model
 * may invoke the stated well-nested single-CPU blocking theorem. */
enum cmx_result sc_init(SCWorld *, const SCConfig *, enum sc_mode);
enum cmx_result sc_lock(SCWorld *, unsigned task, unsigned resource,
                        cmx_deadline, enum sc_context);
enum cmx_result sc_unlock(SCWorld *, unsigned task, unsigned resource,
                          enum sc_context);
/* A blocked acquisition must be polled once after its terminal result.
 * Handoff may own the core mutex before poll records the held-resource bit.
 * No other task operation is admitted while that result remains uncollected. */
enum cmx_result sc_poll(SCWorld *, unsigned task);
enum cmx_result sc_cancel(SCWorld *, unsigned task);
enum cmx_result sc_tick(SCWorld *, uint32_t now);
enum cmx_result sc_set_base(SCWorld *, unsigned task, unsigned priority);
int sc_invariant(const SCWorld *);
/* Stable-current fixed-priority choice for this finite model. A newly ready
 * task with equal effective priority does not preempt the current task.
 * The retained CMX round-robin tie scheduler is a different policy. */
int sc_choose(const SCWorld *, unsigned released_ready_mask, int current);
#endif
