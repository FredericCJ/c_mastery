#ifndef CMASTERY_REALTIME_RESPONSE_H
#define CMASTERY_REALTIME_RESPONSE_H
#include <stdint.h>
#define RTA_TASKS 16u
#define RTA_STEPS 128u
enum rta_status { RTA_CONVERGED,RTA_EXCEEDED,RTA_LIMIT,RTA_INVALID,RTA_OVERFLOW,RTA_UNIMPLEMENTED };
struct rta_task { uint64_t cost,period,deadline,jitter,blocking,overhead; };
struct rta_result { uint64_t estimate,trace[RTA_STEPS+1];unsigned iterations; };
/* Tasks are already ordered by fixed priority; only higher priorities
 * interfere. Input/result storage is disjoint and live throughout the call.
 * INVALID/OVERFLOW preserve result, other statuses publish a complete trace.
 * Zero costs are permitted; a positive target deadline/period is required. */
enum rta_status rta_response(const struct rta_task *,unsigned count,unsigned target,unsigned limit,struct rta_result *);
/* Computes ceil((a+b)/divisor) without overflowing a+b or numerator rounding.
 * Zero divisor or unrepresentable quotient preserves *result. */
enum rta_status rta_ceil_sum(uint64_t,uint64_t,uint64_t,uint64_t *);
#endif
