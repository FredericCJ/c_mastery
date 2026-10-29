#ifndef CMASTERY_TICKLESS_H
#define CMASTERY_TICKLESS_H
#include "cmx.h"
#define TQ_TASKS 4u
enum tq_status { TQ_OK,TQ_INVALID,TQ_FULL,TQ_RETRY,TQ_EMPTY,TQ_UNIMPLEMENTED };
enum tq_relation { TQ_EQUAL,TQ_FUTURE,TQ_PAST,TQ_AMBIGUOUS,TQ_INFINITE };
struct tq_plan { uint64_t token;uint32_t at,observed;unsigned armed; };
struct tq_timer {
 cmx_kernel kernel;uint64_t serial;struct tq_plan plan;
 uint32_t dispatch[TQ_TASKS];unsigned events,prepared,armed;
 uint32_t programmed;
};
struct tq_resume { uint32_t last,numerator,denominator;uint64_t remainder; };
/* Bits8,16,32; raw finite dates require an independently established half-range
 * provenance window. AMBIGUOUS is not future. Infinite is separate state. */
enum tq_status tq_relation(uint32_t now,uint32_t deadline,unsigned bits,unsigned infinite,enum tq_relation *);
/* Rate is numerator/denominator Hz, both1..1,000,000. Results are exact ceilings.
 * Overflow/invalid preserve output; no overflowing intermediate is permitted. */
enum tq_status tq_ticks(uint64_t milliseconds,uint32_t numerator,uint32_t denominator,uint64_t *);
enum tq_status tq_not_before(uint64_t now_us,uint64_t milliseconds,uint32_t numerator,uint32_t denominator,uint64_t *absolute_tick);
enum tq_status tq_init(struct tq_timer *,uint32_t now);
enum tq_status tq_wait(struct tq_timer *,unsigned task,uint32_t delay,unsigned infinite);
enum tq_status tq_cancel(struct tq_timer *,unsigned task);
enum tq_status tq_advance(struct tq_timer *,uint32_t now);
enum tq_status tq_collect(struct tq_timer *,unsigned task,uint32_t *when,enum cmx_result *);
/* Exactly one outstanding plan. Queue mutation may occur between plan/commit
 * under external serialization. Hardware must be disarmed on RETRY/abort.
 * Commit observes time after programming and a pending interrupt snapshot. */
enum tq_status tq_plan(struct tq_timer *,uint32_t now,struct tq_plan *);
enum tq_status tq_commit(struct tq_timer *,struct tq_plan,uint32_t after,unsigned pending_irq);
enum tq_status tq_abort(struct tq_timer *);
/* Stopped local clock; independent counter advances by <2^31 per observation.
 * Ratio converts always-on counts to local ticks. Fraction is retained. */
enum tq_status tq_resume(struct tq_timer *,struct tq_resume *,uint32_t always_on_now);
#endif
