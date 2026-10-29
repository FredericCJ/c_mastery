#ifndef CMASTERY_REALTIME_CEILING_H
#define CMASTERY_REALTIME_CEILING_H
#include "cmx.h"
#define SC_TASKS 8u
#define SC_RESOURCES 4u
enum sc_mode { SC_INHERITANCE, SC_IMMEDIATE };
enum sc_context { SC_TASK_CONTEXT, SC_INTERRUPT_CONTEXT };
struct sc_config {
  unsigned tasks, resources, priority[SC_TASKS], users[SC_RESOURCES],
      ceiling[SC_RESOURCES], strict_nesting;
};
struct sc_world {
  cmx_kernel kernel;
  cmx_mutex mutex[SC_RESOURCES];
  struct sc_config config;
  unsigned held[SC_TASKS], pending[SC_TASKS];
  enum sc_mode mode;
};
/* Stable externally serialized model storage; caller never mutates fields.
 * Initialize fresh storage only; no retained pointer survives destruction.
 * Init is transactional. Configuration and task base priorities are frozen.
 * Resource rank is its index. Acquisitions are strictly increasing; optional
 * strict_nesting also requires reverse release order. Only the latter model
 * may invoke the stated well-nested single-CPU blocking theorem. */
enum cmx_result sc_init(struct sc_world *, const struct sc_config *, enum sc_mode);
enum cmx_result sc_lock(struct sc_world *, unsigned task, unsigned resource,
                        cmx_deadline, enum sc_context);
enum cmx_result sc_unlock(struct sc_world *, unsigned task, unsigned resource,
                          enum sc_context);
/* A blocked acquisition must be polled once after its terminal result.
 * Handoff may own the core mutex before poll records the held-resource bit.
 * No other task operation is admitted while that result remains uncollected. */
enum cmx_result sc_poll(struct sc_world *, unsigned task);
enum cmx_result sc_cancel(struct sc_world *, unsigned task);
enum cmx_result sc_tick(struct sc_world *, uint32_t now);
enum cmx_result sc_set_base(struct sc_world *, unsigned task, unsigned priority);
int sc_invariant(const struct sc_world *);
/* Stable-current fixed-priority choice for this finite model. A newly ready
 * task with equal effective priority does not preempt the current task.
 * The retained CMX round-robin tie scheduler is a different policy. */
int sc_choose(const struct sc_world *, unsigned released_ready_mask, int current);
#endif
