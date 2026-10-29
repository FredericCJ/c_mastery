#ifndef CMASTERY_REALTIME_COHORT_H
#define CMASTERY_REALTIME_COHORT_H
#include "cmx.h"
#define SU_UNITS 32u

struct su_unit
{
    uint32_t due, anchor;
    unsigned active;
};

struct su_state
{
    unsigned q, overrun, active;
    uint32_t period, last;
    uint64_t served, borrowed;
    struct su_unit unit[SU_UNITS];
};
/* Original finite unit-cohort model, no implied POSIX/RTOS implementation.
 * Nonwrapping time. Each call is transactional; malformed state is BROKEN.
 * An active priority interval includes higher-priority preemption. */
enum cmx_result su_init(struct su_state *, unsigned q, uint32_t period,
                        uint32_t start, unsigned overrun);
enum cmx_result su_active(struct su_state *, uint32_t now, unsigned active);
/* Observe n consecutive server execution ticks. At least one unit must be
 * eligible at admission, and n <= available + declared enforcement overrun.
 * Borrowed service is charged against the earliest future unit; it never
 * increases total units. This accounts for overrun, not preventing the first. */
enum cmx_result su_run(struct su_state *, uint32_t now, unsigned n);
unsigned su_available(const struct su_state *);
int su_invariant(const struct su_state *);
#endif
