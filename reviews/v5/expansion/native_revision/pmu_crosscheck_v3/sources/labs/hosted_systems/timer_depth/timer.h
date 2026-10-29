#ifndef CMASTERY_TIMER_DEPTH_H
#define CMASTERY_TIMER_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum TdState { TD_IDLE, TD_ARMED, TD_RUNNING, TD_CANCELLING, TD_RETIRED };
typedef struct {
  unsigned generation, limit;
  enum TdState state;
  uint64_t deadline, period, next;
  int skip, exhausted;
} TdTimer;
typedef struct {
  unsigned generation;
  uint64_t scheduled, expirations, effects, next;
  int exhausted;
} TdDispatch;

/* Serialized by the caller. See CONTRACTS.md for ownership and return values.
 */
int td_init(TdTimer *, unsigned generation_limit);
int td_arm(TdTimer *, uint64_t deadline, uint64_t period, int skip,
           unsigned *generation);
int td_begin(TdTimer *, unsigned generation, uint64_t now, TdDispatch *);
int td_finish(TdTimer *, unsigned generation);
int td_cancel(TdTimer *);
int td_earliest(const TdTimer *, size_t count, size_t *index,
                uint64_t *deadline);
#endif
