#ifndef CMASTERY_TIMER_DEPTH_H
#define CMASTERY_TIMER_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum td_state { TD_IDLE, TD_ARMED, TD_RUNNING, TD_CANCELLING, TD_RETIRED };
struct td_timer {
  unsigned generation, limit;
  enum td_state state;
  uint64_t deadline, period, next;
  int skip, exhausted;
};
struct td_dispatch {
  unsigned generation;
  uint64_t scheduled, expirations, effects, next;
  int exhausted;
};

/* Serialized by the caller. See CONTRACTS.md for ownership and return values.
 */
int td_init(struct td_timer *, unsigned generation_limit);
int td_arm(struct td_timer *, uint64_t deadline, uint64_t period, int skip,
           unsigned *generation);
int td_begin(struct td_timer *, unsigned generation, uint64_t now, struct td_dispatch *);
int td_finish(struct td_timer *, unsigned generation);
int td_cancel(struct td_timer *);
int td_earliest(const struct td_timer *, size_t count, size_t *index,
                uint64_t *deadline);
#endif
