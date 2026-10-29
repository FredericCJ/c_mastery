#include "response.h"
#ifndef RTA_MUTANT
#define RTA_MUTANT 0
#endif
static int add(uint64_t a, uint64_t b, uint64_t *out)
{
    if (b > UINT64_MAX - a)
    {
        return 0;
    }
    *out = a + b;
    return 1;
}

static int multiply(uint64_t a, uint64_t b, uint64_t *out)
{
    uint64_t result = 0;
    while (b)
    {
        if ((b & 1) && !add(result, a, &result))
        {
            return 0;
        }
        b >>= 1;
        if (b)
        {
            if (a > UINT64_MAX / 2)
            {
                return 0;
            }
            a <<= 1;
        }
    }
    *out = result;
    return 1;
}

/* Restoring division: high preserves the65th bit of2*remainder+input_bit.
 * Before every iteration remainder<divisor; subtraction restores it. */
static uint64_t divide(uint64_t numerator, uint64_t divisor,
                       uint64_t *remainder)
{
    uint64_t quotient = 0, rem = 0;
    for (unsigned bit = 64; bit;)
    {
        bit--;
        uint64_t high = rem >> 63;
        rem = (rem << 1) | ((numerator >> bit) & 1);
        if (high || rem >= divisor)
        {
            rem -= divisor;
            quotient |= UINT64_C(1) << bit;
        }
    }
    *remainder = rem;
    return quotient;
}

enum rta_status rta_ceil_sum(uint64_t a, uint64_t b, uint64_t divisor,
                             uint64_t *out)
{
    if (!divisor || !out)
    {
        return RTA_INVALID;
    }
    uint64_t ar, br, aq = divide(a, divisor, &ar), bq = divide(b, divisor, &br),
                     whole, rem;
    if (!add(aq, bq, &whole))
    {
        return RTA_OVERFLOW;
    }
    if (ar >= divisor - br)
    {
        if (!add(whole, 1, &whole))
        {
            return RTA_OVERFLOW;
        }
        rem = ar - (divisor - br);
    }
    else
    {
        rem = ar + br;
    }
    if (rem && RTA_MUTANT != 1 && !add(whole, 1, &whole))
    {
        return RTA_OVERFLOW;
    }
    *out = whole;
    return RTA_CONVERGED;
}

enum rta_status rta_response(const struct rta_task *tasks, unsigned count,
                             unsigned target, unsigned limit,
                             struct rta_result *out)
{
    if (RTA_MUTANT == 6 && out)
    {
        out->trace[0] = 0;
    }
    if (!tasks || !out || !count || count > RTA_TASKS || target >= count ||
        !limit || limit > RTA_STEPS)
    {
        return RTA_INVALID;
    }
    for (unsigned i = 0; i < count; i++)
    {
        if (!tasks[i].period || !tasks[i].deadline)
        {
            return RTA_INVALID;
        }
    }
    uint64_t base;
    if (!add(tasks[target].cost, tasks[target].overhead, &base) ||
        !add(base, RTA_MUTANT == 2 ? 0 : tasks[target].blocking, &base))
    {
        return RTA_OVERFLOW;
    }
    struct rta_result result = {.estimate = base};
    result.trace[0] = base;
    if (!base)
    {
        *out = result;
        return RTA_CONVERGED;
    }
    if (base > tasks[target].deadline)
    {
        *out = result;
        return RTA_EXCEEDED;
    }
    for (unsigned iteration = 0; iteration < limit; iteration++)
    {
        uint64_t next = base;
        for (unsigned h = 0; h < target; h++)
        {
            uint64_t arrivals, cost, work;
            enum rta_status status = rta_ceil_sum(
                result.estimate, RTA_MUTANT == 4 ? 0 : tasks[h].jitter,
                tasks[h].period, &arrivals);
            if (status != RTA_CONVERGED)
            {
                return status;
            }
            if (!add(tasks[h].cost, RTA_MUTANT == 5 ? 0 : tasks[h].overhead,
                     &cost) ||
                !multiply(arrivals, cost, &work) || !add(next, work, &next))
            {
                return RTA_OVERFLOW;
            }
        }
        result.iterations = iteration + 1;
        result.trace[result.iterations] = next;
        uint64_t previous = result.estimate;
        result.estimate = next;
        if (next > tasks[target].deadline)
        {
            *out = result;
            return RTA_EXCEEDED;
        }
        if (next == previous)
        {
            *out = result;
            return RTA_CONVERGED;
        }
    }
    *out = result;
    return RTA_MUTANT == 3 ? RTA_CONVERGED : RTA_LIMIT;
}
